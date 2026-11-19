import { createToolWorker } from "./browser-tools.js";

const api = "/mctools/api/modpack";
const modrinth = "https://api.modrinth.com/v2";

async function json(response, error) {
    const data = await response.json().catch(() => null);
    if (!response.ok || !data) throw new Error(data?.error || `${error} (HTTP ${response.status})`);
    return data;
}

function post(path, body) {
    return fetch(api + path, {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify(body),
    });
}

async function upstream(url) {
    return json(await fetch(url, { signal: AbortSignal.timeout(5000) }), "Сервис недоступен");
}

let releases;
export async function minecraftVersions(loader) {
    try {
        releases ??= upstream("https://launchermeta.mojang.com/mc/game/version_manifest.json")
            .then(({ versions }) => versions.filter(({ type }) => type === "release"))
            .catch((error) => {
                releases = undefined;
                throw error;
            });
        return await releases;
    } catch {
        return (await json(await post("/versions/mc", { loader }), "Не удалось получить версии Minecraft")).versions;
    }
}

export async function loaderVersions(loader, mc_version) {
    if (loader !== "fabric") return [];
    try {
        const versions = await upstream(`https://meta.fabricmc.net/v2/versions/loader/${encodeURIComponent(mc_version)}`);
        return versions.map(({ loader }) => ({ version: loader.version, is_stable: loader.stable }));
    } catch {
        return (await json(await post("/versions/loaders", { loader, mc_version }), "Не удалось получить версии загрузчика")).versions;
    }
}

export async function findMods(query, loader, mc_version) {
    const limit = 20;
    const facets = JSON.stringify([["project_type:mod"], [`categories:${loader}`], [`versions:${mc_version}`]]);
    try {
        const { hits } = await upstream(`${modrinth}/search?${new URLSearchParams({ query, facets, limit })}`);
        return hits.filter((hit) => hit.project_id).map((hit) => ({ ...hit, id: hit.project_id }));
    } catch {
        return (await json(await post("/mods/search", { query, loader, mc_version, limit }), "Ошибка поиска")).mods;
    }
}

export async function exportPack(request, onFallback) {
    let worker;
    try {
        worker = createToolWorker();
    } catch {
        // Fall back to a server-built archive.
    }
    if (worker) {
        try {
            const plan = await json(await post("/export?prepare=1", request), "Не удалось подготовить сборку");
            try {
                const bytes = await worker.run("archive", plan);
                return { blob: new Blob([bytes], { type: "application/zip" }), filename: plan.filename,
                    skipped: plan.warnings.join("; "), local: true };
            } catch {
                // Retry the failed browser build on the server.
            }
        } finally {
            worker.close();
        }
    }

    onFallback();
    const response = await post("/export", request);
    if (!response.ok) await json(response, "Не удалось собрать сборку");
    const disposition = response.headers.get("Content-Disposition") || "";
    const match = /filename\*?=(?:UTF-8'')?"?([^";]+)"?/i.exec(disposition);
    return { blob: await response.blob(), filename: match ? decodeURIComponent(match[1]) : "mctools-modpack.zip",
        skipped: response.headers.get("X-MCTools-Skipped-Mods"), local: false };
}
