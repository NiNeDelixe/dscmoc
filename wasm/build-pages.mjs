import assert from "node:assert/strict";
import { cp, mkdir, readFile, writeFile } from "node:fs/promises";
import { posix } from "node:path";

const root = "backend/projects";
const out = "out/wasm-pages";
const home = "HomeSite/views/HomePageView";
const mc = "MCTools/views";
const pages = {
    "/": home,
    "/about": home,
    "/contact": home,
    "/contacts": home,
    "/projects": home,
    "/noise": "NoiseGenerator/views/NoisePageView",
    "/mctools": `${mc}/MCToolsView`,
    "/mctools/templates": `${mc}/prj_template/prj_template`,
    "/mctools/packs": `${mc}/packs`,
    "/mctools/skin_viewer": `${mc}/skin_viewer`,
    "/mctools/tools": `${mc}/tools`,
};

async function render(view) {
    let html = await readFile(`${root}/${view}.html`, "utf8");
    for (const [tag, name] of html.matchAll(/<%view\s+(\w+)(?:\.csp)?\s*%>/g)) {
        const dir = name === "header" || name === "footer" ? mc : posix.dirname(view);
        const fragment = await render(`${dir}/${name}`);
        html = html.replace(tag, () => fragment);
    }
    return html;
}

await mkdir(out, { recursive: true });
for (const dir of ["css", "images", "js", "wasm"]) {
    await cp(`static/${dir}`, `${out}/static/${dir}`, { recursive: true });
}
await cp(`${root}/MCTools/data/mods.json`, `${out}/static/mods.json`);
await cp("static/tools.json", `${out}/static/tools.json`);
for (const [route, view] of Object.entries(pages)) {
    const folder = `${out}${route === "/" ? "" : route}`;
    const base = route === "/" ? "./" : "../".repeat(route.split("/").length - 1);
    const html = (await render(view))
        .replace("[[ title ]]", "Конструктор модпака")
        .replace(/ data-noise-endpoint="[^"]+"/, "")
        .replace("<body", `<body data-static-site="${base}"`)
        .replace(/\b(href|src)="(\/[^"/][^"]*|\/)"/g, (tag, attr, url) => {
            if (url.startsWith("/static/")) return `${attr}="${base}${url.slice(1)}"`;
            assert.ok(Object.hasOwn(pages, url), `Unknown page link ${url} in ${view}`);
            return `${attr}="${base}${url === "/" ? "" : url.slice(1) + "/"}"`;
        });
    assert.ok(!html.includes("<%") && !html.includes("[["), `Unresolved template in ${view}`);
    await mkdir(folder, { recursive: true });
    await writeFile(`${folder}/index.html`, html);
}
