import { strict as assert } from "node:assert";
import { createHash } from "node:crypto";

globalThis.self = {};
await import("../static/js/tools-worker.js");

function run(tool, args) {
    return new Promise((resolve, reject) => {
        self.postMessage = ({ bytes, error }) => error ? reject(new Error(error)) : resolve(Buffer.from(bytes));
        self.onmessage({ data: { id: 1, tool, args } });
    });
}

const args = [64, -40, 12, 4, 0.05, 0.5, 7];
const bmp = await run("noise", args);
assert.equal(bmp.toString("ascii", 0, 2), "BM");
assert.equal(bmp.readInt32LE(18), 64);
assert.equal(bmp.readInt32LE(22), 64);
assert.deepEqual(await run("noise", args), bmp);
for (const [index, value] of [[1, 0], [3, 1], [4, 0.2], [5, 1]]) {
    const changed = args.with(index, value);
    assert.notDeepEqual(await run("noise", changed), bmp);
}
await assert.rejects(run("noise", args.with(0, 1001)), /Не удалось создать карту/);

const jar = Buffer.from("example mod bytes");
const hash = createHash("sha512").update(jar);
const integrity = hash.copy().digest("base64");
const file = { path: "mods/example.jar", size: jar.length, sha512: hash.digest("hex"),
    url: "https://cdn.modrinth.com/example.jar" };
globalThis.fetch = async (url, options) => {
    assert.equal(url, file.url);
    assert.equal(options.integrity, `sha512-${integrity}`);
    return new Response(jar);
};

for (const documents of [
    [["modrinth.index.json", '{"formatVersion":1}']],
    [["README.txt", "Инструкция"]],
    [["pakku.json", 'game = "minecraft"'], ["README.txt", "Инструкция"]],
]) {
    const files = documents[0][0] === "modrinth.index.json" ? [] : [file];
    const zip = await run("archive", { documents, files });
    let offset = 0;
    const expected = [...documents.map(([name, text]) => [name, Buffer.from(text)]),
        ...files.map(({ path }) => [path, jar])];
    for (const [name, bytes] of expected) {
        assert.equal(zip.readUInt32LE(offset), 0x04034b50);
        assert.equal(zip.readUInt32LE(offset + 18), bytes.length);
        const nameStart = offset + 30;
        const nameEnd = nameStart + zip.readUInt16LE(offset + 26);
        assert.equal(zip.toString("ascii", nameStart, nameEnd), name);
        const dataStart = nameEnd + zip.readUInt16LE(offset + 28);
        offset = dataStart + bytes.length;
        assert.deepEqual(zip.subarray(dataStart, offset), bytes);
    }
    assert.equal(zip.readUInt32LE(offset), 0x02014b50);
    assert.equal(zip.readUInt32LE(zip.length - 22), 0x06054b50);
}
await assert.rejects(run("archive", { documents: [], files: [{ ...file, size: 2 ** 28 }] }), /лимит/);
await assert.rejects(run("archive", { documents: [], files: [{ ...file, path: "mods/../bad.jar" }] }),
    /Некорректный файл/);
assert.equal((await run("archive", { documents: [["README.txt", "ok"]], files: [] })).readUInt32LE(0),
    0x04034b50);
console.log("WASM smoke check passed");
