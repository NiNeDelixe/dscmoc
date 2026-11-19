const loaderKeys = { fabric: "fabric-loader", forge: "forge", neoforge: "neoforge" };
const formats = { "mods-zip": "-mods.zip", mrpack: ".mrpack", "pakku-project": "-pakku-project.zip" };
const safeName = (name) => name.replace(/[^a-z0-9_.-]/gi, "_");
const toml = (value) => JSON.stringify(value.toWellFormed()).replaceAll("\x7f", "\\u007f");

export async function preparePack(request, versionsFor) {
    const { loader, mc_version, loader_version, format, mods, instructions, java_args } = request;
    if (!Object.hasOwn(loaderKeys, loader) || !mc_version || !mods.length) {
        throw new Error("Выберите загрузчик, версию Minecraft и хотя бы один мод");
    }
    if (!Object.hasOwn(formats, format)) throw new Error("Неизвестный формат экспорта");

    const files = [];
    const warnings = [];
    const paths = new Set();
    for (const mod of mods) {
        const title = mod.title || mod.slug || mod.id;
        const versions = await versionsFor(mod.id, loader, mc_version);
        let found = false;
        for (const version of versions) {
            const jars = (version.files || []).filter((file) =>
                /.+\.jar$/i.test(file.filename) &&
                typeof file.url === "string" && file.url.startsWith("https://cdn.modrinth.com/") &&
                /^[a-f0-9]{40}$/i.test(file.hashes?.sha1) &&
                /^[a-f0-9]{128}$/i.test(file.hashes?.sha512) &&
                Number.isSafeInteger(file.size) && file.size > 0);
            const file = jars.find((jar) => jar.primary) || jars[0];
            if (!file) continue;

            const path = `mods/${safeName(file.filename)}`;
            if (paths.has(path.toLowerCase())) throw new Error(`Повторяющийся файл: ${path}`);
            paths.add(path.toLowerCase());
            files.push({ path, url: file.url, size: file.size,
                sha1: file.hashes.sha1, sha512: file.hashes.sha512,
                title, modId: mod.id, versionId: version.id });
            found = true;
            break;
        }
        if (!found) warnings.push(title);
    }
    if (!files.length) throw new Error(`Ни один из выбранных модов не поддерживает ${loader} ${mc_version}`);

    const name = request.pack_name.trim() || "MCTools";
    const filename = safeName(name).replace(/^\.+/, "") || "mctools-modpack";
    const documents = [];
    if (format === "mrpack") {
        const dependencies = { minecraft: mc_version };
        if (loader_version) dependencies[loaderKeys[loader]] = loader_version;
        const index = {
            formatVersion: 1, game: "minecraft", versionId: name, name, dependencies,
            files: files.map(({ path, sha1, sha512, url, size }) => ({
                path, hashes: { sha1, sha512 }, downloads: [url], fileSize: size,
                env: { client: "required", server: "required" },
            })),
        };
        return { filename: filename + formats[format], documents: [["modrinth.index.json", JSON.stringify(index, null, 2)]],
            files: [], warnings };
    }

    const loaderTitle = loader_version ? `${loader} ${loader_version}` : loader;
    documents.push(["README.txt", [
        name, "===========", "", `Minecraft: ${mc_version}`, `Загрузчик: ${loaderTitle}`,
        `Модов: ${files.length}`, "",
        ...(java_args ? ["Аргументы Java", java_args, ""] : []),
        ...(instructions ? ["Инструкция", instructions, ""] : []),
        "Состав сборки", ...files.map(({ title, path, url }) => `* ${title} — ${path}\n  ${url}`), "",
    ].join("\n")]);

    if (format === "pakku-project") {
        const pakku = [
            `name = ${toml(name)}`, 'game = "minecraft"',
            `minecraft-version = ${toml(mc_version)}`, `mod-loader = ${toml(loader)}`,
            ...(loader_version ? [`mod-loader-version = ${toml(loader_version)}`] : []),
        ];
        for (const file of files) {
            pakku.push("", "[[files]]", `file = ${toml(file.path)}`, `hash = ${toml(file.sha1)}`,
                'hash-format = "sha1"', 'mode = "manual"', `url = ${toml(file.url)}`, "",
                "[files.metadata]", `mod-id = ${toml(file.modId)}`, `version-id = ${toml(file.versionId)}`,
                `title = ${toml(file.title)}`);
        }
        if (java_args) pakku.push("", "[java]", `args = ${toml(java_args)}`);
        documents.unshift(["pakku.json", pakku.join("\n") + "\n"]);
    }
    return { filename: filename + formats[format], documents, files, warnings };
}
