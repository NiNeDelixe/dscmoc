(() => {
    const page = document.body.dataset.page || "";
    const icons = {
        "file-text":
            '<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round" stroke-linejoin="round"><path d="M14 2H6a2 2 0 0 0-2 2v16a2 2 0 0 0 2 2h12a2 2 0 0 0 2-2V8zM14 2v6h6M8 13h8M8 17h8"/></svg>',
        "user-round":
            '<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round"><circle cx="10" cy="8" r="4"/><path d="M3 21v-2a7 7 0 0 1 14 0v2M19 8v6M22 11h-6"/></svg>',
        box: '<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round" stroke-linejoin="round"><path d="m12 2 9 5-9 5-9-5 9-5ZM3 7v10l9 5 9-5V7M12 12v10"/></svg>',
        settings:
            '<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round"><circle cx="12" cy="12" r="3"/><path d="M19 13.5 21 15l-2 4-2.5-1a8 8 0 0 1-2 1L14 21h-4l-.5-2a8 8 0 0 1-2-1L5 19l-2-4 2-1.5v-3L3 9l2-4 2.5 1a8 8 0 0 1 2-1L10 3h4l.5 2a8 8 0 0 1 2 1L19 5l2 4-2 1.5z"/></svg>',
    };
    async function partial(id, url) {
        const el = document.getElementById(id);
        if (!el) return;
        try {
            const r = await fetch(url);
            if (!r.ok) throw Error(r.status);
            el.innerHTML = await r.text();
        } catch (e) {
            console.error("Не удалось загрузить", url, e);
        }
    }
    function csv(text) {
        return text
            .trim()
            .split(/\\r?\\n/)
            .slice(1)
            .map((line) => {
                const p = line.split(",");
                return {
                    id: p[0],
                    title: p[1],
                    description: p[2],
                    url: p[3],
                    icon: p[4],
                };
            })
            .filter((x) => x.id);
    }
    async function cards() {
        const targets = [
            document.getElementById("tool-grid"),
            document.getElementById("additional-tools-list"),
        ].filter(Boolean);
        if (!targets.length) return;
        try {
            const r = await fetch("data/tools.csv");
            const items = csv(await r.text());
            targets.forEach((el) => {
                el.replaceChildren();
                items
                    .filter((x) =>
                        el.id === "additional-tools-list" ? x.id === "tools" : true,
                    )
                    .forEach((x) => {
                        const a = document.createElement("article");
                        a.className = "card tool-card";
                        const i = document.createElement("div");
                        i.className = "tool-icon";
                        i.innerHTML = icons[x.icon] || icons.box;
                        const h = document.createElement("h3");
                        h.textContent = x.title;
                        const p = document.createElement("p");
                        p.textContent =
                            x.description === "Описание из backend" ? "" : x.description;
                        const link = document.createElement("a");
                        link.className = "button";
                        link.href = x.url;
                        link.textContent = "Перейти";
                        a.append(i, h, p, link);
                        el.append(a);
                    });
            });
        } catch (e) {
            console.error("CSV tools не загружен", e);
        }
    }
    function nav() {
        const b = document.getElementById("nav-toggle"),
            n = document.getElementById("nav-links");
        if (!b || !n) return;
        b.addEventListener("click", () => {
            const open = n.classList.toggle("open");
            b.setAttribute("aria-expanded", String(open));
        });
        n.querySelectorAll("[data-nav]").forEach((a) => {
            if (a.dataset.nav === page) a.classList.add("active");
        });
    }
    function wizard() {
        const stages = [...document.querySelectorAll(".wizard-stage")];
        if (!stages.length) return;
        const steps = [...document.querySelectorAll(".step")],
            prev = document.getElementById("prev-step"),
            next = document.getElementById("next-step");
        let current = 0;
        function show(n) {
            current = Math.max(0, Math.min(stages.length - 1, n));
            stages.forEach((x, i) => x.classList.toggle("hidden", i !== current));
            steps.forEach((x, i) => {
                x.classList.toggle("active", i === current);
                x.classList.toggle("done", i < current);
            });
            prev.disabled = current === 0;
            next.classList.toggle("hidden", current === stages.length - 1);
        }
        prev.addEventListener("click", () => show(current - 1));
        next.addEventListener("click", () => show(current + 1));
        steps.forEach((x) =>
            x.addEventListener("click", () => show(Number(x.dataset.step))),
        );
        document
            .getElementById("download-button")
            ?.addEventListener("click", () => {
                /* BACKEND: вызвать Drogon endpoint генерации архива и обработать скачивание. */
            });
        show(0);
    }
    document.addEventListener("DOMContentLoaded", async () => {
        await Promise.all([
            partial("site-header", "partials/header.html"),
            partial("site-footer", "partials/footer.html"),
        ]);
        nav();
        wizard();
        cards();
    });
})();
