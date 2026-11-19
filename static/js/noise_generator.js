import { createToolWorker } from "./browser-tools.js";

const sliders = ["noiseSize", "offsetX", "offsetY", "octaves", "frequency", "persistence"]
    .map((id) => document.getElementById(id));
const imageContainer = document.getElementById("imageContainer");
const save = document.getElementById("saveBtn");
const status = document.getElementById("noiseStatus");
const endpoint = document.body.dataset.noiseEndpoint;
let worker;
try {
    worker = createToolWorker();
} catch {
    // Fall back to server-side generation.
}
let seed = 0;
let timer;
let requestId = 0;
let currentUrl;

async function generate(id) {
    const args = [...sliders.map((slider) => Number(slider.value)), seed];
    status.textContent = "Генерируем карту...";

    try {
        let blob;
        let local = false;
        if (worker) {
            try {
                const bytes = await worker.run("noise", args);
                blob = new Blob([bytes], { type: "image/bmp" });
                local = true;
            } catch {
                worker?.close();
                worker = undefined;
            }
        }
        if (id !== requestId) return;
        if (!blob) {
            if (!endpoint) {
                status.textContent = "Не удалось запустить WebAssembly в браузере";
                return;
            }
            const params = new URLSearchParams(
                ["size", "offsetX", "offsetY", "octaves", "frequency", "persistence", "t"]
                    .map((name, i) => [name, args[i]]));
            const response = await fetch(`${endpoint}?${params}`);
            if (!response.ok) throw new Error("Не удалось создать карту шума");
            blob = await response.blob();
        }
        if (id !== requestId) return;
        if (currentUrl) URL.revokeObjectURL(currentUrl);
        currentUrl = URL.createObjectURL(blob);
        const image = new Image(512, 512);
        image.className = "noise-image";
        image.alt = "Карта шума";
        image.src = currentUrl;
        imageContainer.replaceChildren(image);
        save.disabled = false;
        status.textContent = local ? "Карта создана в браузере" : "Карта создана на сервере";
    } catch {
        if (id === requestId) status.textContent = "Не удалось создать карту шума";
    }
}

for (const slider of sliders) {
    slider.addEventListener("input", () => {
        document.getElementById(`${slider.id}Value`).textContent = slider.value;
        clearTimeout(timer);
        const id = ++requestId;
        timer = setTimeout(() => generate(id), 500);
    });
}

function newMap() {
    clearTimeout(timer);
    seed = Date.now() | 0;
    generate(++requestId);
}

document.getElementById("generateBtn").addEventListener("click", newMap);
save.addEventListener("click", () => {
    const link = document.createElement("a");
    link.href = currentUrl;
    link.download = "noise_image.bmp";
    link.click();
});
newMap();
