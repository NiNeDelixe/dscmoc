import assert from "node:assert/strict";
import { cp, mkdir, readFile, writeFile } from "node:fs/promises";

const views = "backend/projects/NoiseGenerator/views";
const out = "out/wasm-pages";
const view = await readFile(`${views}/NoisePageView.html`, "utf8");
const controls = await readFile(`${views}/NoisePageResources.html`, "utf8");
const partial = "<%view NoisePageResources.csp%>";
assert.ok(view.includes(partial), "Noise page no longer includes its controls");

const html = view.replace(partial, () => controls)
    .replace(/ data-noise-endpoint="[^"]+"/, "")
    .replaceAll('="/static/', '="./static/');
assert.ok(!html.includes("<%"), "Unresolved Drogon template in the Pages demo");

await mkdir(out, { recursive: true });
for (const asset of [
    "css/noise.css",
    "images/powered_by_libnoise.svg",
    "js/browser-tools.js",
    "js/noise_generator.js",
    "js/tools-worker.js",
    "wasm",
]) {
    await cp(`static/${asset}`, `${out}/static/${asset}`, { recursive: true });
}
await writeFile(`${out}/index.html`, html);
