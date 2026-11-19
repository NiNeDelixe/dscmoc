export function createToolWorker() {
    const worker = new Worker(new URL("./tools-worker.js", import.meta.url), { type: "module" });
    const pending = new Map();
    let nextId = 0;
    let failure;

    worker.onmessage = ({ data: { id, bytes, error } }) => {
        const request = pending.get(id);
        pending.delete(id);
        if (error) request?.reject(new Error(error));
        else request?.resolve(bytes);
    };

    function close(error = new Error("WebAssembly недоступен")) {
        failure = error;
        worker.terminate();
        for (const { reject } of pending.values()) reject(error);
        pending.clear();
    }

    worker.onerror = () => close();

    return {
        run(tool, args) {
            if (failure) return Promise.reject(failure);
            return new Promise((resolve, reject) => {
                const id = ++nextId;
                worker.postMessage({ id, tool, args });
                pending.set(id, { resolve, reject });
            });
        },
        close,
    };
}
