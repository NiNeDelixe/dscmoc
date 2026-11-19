import createModule from "../wasm/tools.js";

let runtime;
let queue = Promise.resolve();

function takeFile(wasm, path) {
    try {
        return wasm.FS.readFile(path);
    } finally {
        wasm.FS.unlink(path);
    }
}

async function buildArchive(wasm, { documents, files }) {
    const maxBytes = 128 * 1024 * 1024;
    if (files.some(({ size }) => !Number.isSafeInteger(size) || size <= 0) ||
        files.reduce((sum, { size }) => sum + size, 0) > maxBytes) {
        throw new Error("Размер сборки превышает лимит браузера");
    }

    const openEntry = wasm.cwrap("open_zip_entry", "number", ["number", "string"]);
    const archive = wasm._open_zip();
    if (!archive) throw new Error("Не удалось создать архив");

    const names = new Set();
    const check = (code) => {
        if (code !== 0) throw new Error(`Ошибка записи архива: ${code}`);
    };
    function add(path, bytes) {
        if (names.has(path.toLowerCase())) throw new Error(`Повторяющийся файл: ${path}`);
        names.add(path.toLowerCase());
        check(openEntry(archive, path));
        if (bytes.length) {
            const buffer = wasm._malloc(bytes.length);
            if (!buffer) throw new Error("Недостаточно памяти для сборки");
            try {
                wasm.HEAPU8.set(bytes, buffer);
                check(wasm._zipWriteInFileInZip(archive, buffer, bytes.length));
            } finally {
                wasm._free(buffer);
            }
        }
        check(wasm._zipCloseFileInZip(archive));
    }

    try {
        const encoder = new TextEncoder();
        for (const [path, text] of documents) add(path, encoder.encode(text));
        for (const { path, url, size, sha512 } of files) {
            if (!/^mods\/[a-z0-9_.-]+\.jar$/i.test(path) ||
                new URL(url).origin !== "https://cdn.modrinth.com" ||
                !/^[a-f0-9]{128}$/i.test(sha512)) {
                throw new Error("Некорректный файл Modrinth");
            }
            const hash = sha512.match(/../g).map((hex) => String.fromCharCode(parseInt(hex, 16))).join("");
            const response = await fetch(url, {
                integrity: `sha512-${btoa(hash)}`,
                credentials: "omit",
                redirect: "error",
                signal: AbortSignal.timeout(60000),
            });
            if (!response.ok) throw new Error(`Не удалось скачать ${path}`);
            const bytes = new Uint8Array(await response.arrayBuffer());
            if (bytes.length !== size) throw new Error(`Неверный размер файла ${path}`);
            add(path, bytes);
        }
    } catch (error) {
        wasm._zipClose(archive, 0);
        wasm.FS.unlink("/pack.zip");
        throw error;
    }
    const closeCode = wasm._zipClose(archive, 0);
    const bytes = takeFile(wasm, "/pack.zip");
    check(closeCode);
    return bytes;
}

self.onmessage = ({ data: { id, tool, args } }) => {
    queue = queue.then(async () => {
        try {
            const wasm = await (runtime ??= createModule());
            let bytes;
            if (tool === "noise") {
                if (!wasm._write_noise(...args)) throw new Error("Не удалось создать карту шума");
                bytes = takeFile(wasm, "/noise.bmp");
            } else {
                bytes = await buildArchive(wasm, args);
            }
            self.postMessage({ id, bytes }, [bytes.buffer]);
        } catch (error) {
            self.postMessage({ id, error: error.message || "Ошибка WebAssembly" });
        }
    });
};
