import assert from "node:assert/strict";
import { readFile } from "node:fs/promises";
import { preparePack } from "../static/js/modpack-plan.js";

const root = "out/wasm-pages";
const routes = ["", "projects", "about", "contacts", "noise", "mctools", "mctools/templates",
    "mctools/packs", "mctools/skin_viewer", "mctools/tools"];
for (const route of routes) {
    const html = await readFile(`${root}/${route ? `${route}/` : ""}index.html`, "utf8");
    const page = new URL(`${route ? `${route}/` : ""}`, "https://example.org/dscmoc/develop/");
    for (const [match, value] of html.matchAll(/\b(?:href|src)="([^"#]+)"/g)) {
        if (value.startsWith("https://")) continue;
        const url = new URL(value, page);
        assert.ok(url.pathname.startsWith("/dscmoc/develop/"), `Escapes deployment path: ${match}`);
        const local = url.pathname.slice("/dscmoc/develop/".length);
        await readFile(`${root}/${local}${url.pathname.endsWith("/") ? "index.html" : ""}`);
    }
}

const request = {
    loader: "fabric", mc_version: "1.20.1", loader_version: "0.16.14",
    pack_name: 'Demo "Pack"', mods: [{ id: "test", title: "Мод" }, { id: "skip", title: "Пропуск" }],
    java_args: "-Xmx4G", instructions: "Установите Fabric",
};
const jar = { filename: "mod+1.jar", url: "https://cdn.modrinth.com/data/test/mod.jar",
    hashes: { sha1: "a".repeat(40), sha512: "b".repeat(128) }, size: 1024 };
const versionsFor = async (id) => id === "test" ? [
    { id: "bad", files: [{ ...jar, url: "https://elsewhere.test/bad.jar" }] },
    { id: "good", files: [{ ...jar, filename: "extra.jar" }, { ...jar, primary: true }] },
] : [];
const mrpack = await preparePack({ ...request, format: "mrpack" }, versionsFor);
assert.equal(mrpack.files.length, 0);
assert.deepEqual(mrpack.warnings, ["Пропуск"]);
const index = JSON.parse(mrpack.documents[0][1]);
assert.equal(index.files[0].path, "mods/mod_1.jar");
assert.deepEqual(index.dependencies, { minecraft: "1.20.1", "fabric-loader": "0.16.14" });
const zip = await preparePack({ ...request, format: "mods-zip" }, versionsFor);
assert.deepEqual(zip.documents.map(([path]) => path), ["README.txt"]);
assert.equal(zip.files[0].sha512, jar.hashes.sha512);
const pakku = await preparePack({ ...request, format: "pakku-project" }, versionsFor);
assert.deepEqual(pakku.documents.map(([path]) => path), ["pakku.json", "README.txt"]);
assert.ok(pakku.documents[0][1].includes(String.raw`name = "Demo \"Pack\""`));
assert.ok(pakku.documents[1][1].includes(request.instructions));
await assert.rejects(preparePack({ ...request, format: "mrpack", mods: [request.mods[1]] }, versionsFor),
    /не поддерживает/);
console.log("Static routes, links, and export plans passed");
