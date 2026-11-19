(() => {
    const $ = (sel, root = document) => root.querySelector(sel);
    const $$ = (sel, root = document) => [...root.querySelectorAll(sel)];

    const icons = {
        "file-text":
            '<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round" stroke-linejoin="round"><path d="M14 2H6a2 2 0 0 0-2 2v16a2 2 0 0 0 2 2h12a2 2 0 0 0 2-2V8zM14 2v6h6M8 13h8M8 17h8"/></svg>',
        "user-round":
            '<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round"><circle cx="10" cy="8" r="4"/><path d="M3 21v-2a7 7 0 0 1 14 0v2M19 8v6M22 11h-6"/></svg>',
        box: '<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round" stroke-linejoin="round"><path d="m12 2 9 5-9 5-9-5 9-5ZM3 7v10l9 5 9-5V7M12 12v10"/></svg>',
        settings:
            '<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round"><circle cx="12" cy="12" r="3"/><path d="M19 13.5 21 15l-2 4-2.5-1a8 8 0 0 1-2 1L14 21h-4l-.5-2a8 8 0 0 1-2-1L5 19l-2-4 2-1.5v-3L3 9l2-4 2.5 1a8 8 0 0 1 2-1L10 3h4l.5 2a8 8 0 0 1 2 1L19 5l2 4-2 1.5z"/></svg>',
    };

    async function cards() {
        const homeGrid = $("#tool-grid");
        const extraGrid = $("#additional-tools-list");
        if (!homeGrid && !extraGrid) return;

        try {
            const base = document.body.dataset.staticSite;
            const r = await fetch(base ? `${base}static/tools.json` : "/mctools/api/tools");
            if (!r.ok) throw Error(r.status);
            const data = await r.json();
            const all = data.tools || [];
            const extra = data.additional || [];

            function render(grid, items) {
                if (!grid) return;
                grid.replaceChildren();
                items.forEach((x) => {
                    const a = document.createElement("article");
                    a.className = "card tool-card";

                    const i = document.createElement("div");
                    i.className = "tool-icon";
                    i.innerHTML = icons[x.icon] || icons.box;

                    const h = document.createElement("h3");
                    h.textContent = x.title || x.id;

                    const p = document.createElement("p");
                    p.textContent = x.description || "";

                    const link = document.createElement("a");
                    link.className = "button";
                    link.href = base ? `${base}${x.url.slice(1)}/` : x.url;
                    link.textContent = "Перейти";

                    a.append(i, h, p, link);
                    grid.append(a);
                });
            }

            render(homeGrid, all);
            render(extraGrid, extra);
        } catch (e) {
            console.error("Не удалось загрузить список инструментов", e);
        }
    }

    function nav() {
        const b = $("#nav-toggle"), n = $("#nav-links");
        if (!b || !n) return;
        b.addEventListener("click", () => {
            const open = n.classList.toggle("open");
            b.setAttribute("aria-expanded", String(open));
        });
        $$("[data-nav]", n).forEach((a) => {
            if (a.dataset.nav === document.body.dataset.page) a.classList.add("active");
        });
    }

    const instructionsText = {
        fabric: "Установи Fabric Loader нужной версии, создай новый профиль и положи "
            + "содержимое архива в его папку mods/. Запускай профиль через лаунчер Fabric.",
        forge: "Установи Forge нужной версии, создай новый профиль и положи содержимое "
            + "архива в его папку mods/. Запускай профиль через лаунчер Forge.",
        neoforge: "Установи NeoForge нужной версии, создай новый профиль и положи "
            + "содержимое архива в его папку mods/. Запускай профиль через лаунчер NeoForge.",
    };

    const formatHints = {
        "mods-zip": "В архиве будут лежать сами .jar в каталоге mods/ и файл README.txt "
            + "с составом сборки. Подходит, если ты хочешь разобрать сборку вручную.",
        "mrpack": "В архив попадёт только modrinth.index.json: лаунчер сам скачает моды по "
            + "ссылкам и проверит хэши. Файл импортируется в Modrinth и PrismaLauncher.",
        "pakku-project": "В архиве будут pakku.json, README.txt и каталог mods/ с модами — "
            + "структура, которую понимает Pakku.",
    };

    function wizard() {
        const stages = $$(".wizard-stage");
        if (!stages.length) return;

        const steps = $$(".step"),
            prev = $("#prev-step"),
            next = $("#next-step");

        const mcVersionSelect = $("#mc-version"),
            loaderVersionSelect = $("#loader-version"),
            catalogSearch = $("#catalog-search"),
            catalogStatus = $("#catalog-status"),
            modList = $("#mod-list"),
            searchInput = $("#mod-search"),
            searchButton = $("#mod-search-button"),
            searchStatus = $("#mod-search-status"),
            searchResults = $("#search-results"),
            selectionSummary = $("#mod-selection-summary"),
            exportFormatHint = $("#export-format-hint"),
            instructionSelect = $("#instructions"),
            instructionContent = $("#instruction-content"),
            packNameInput = $("#pack-name"),
            javaArgsInput = $("#java-args"),
            downloadButton = $("#download-button");

        const selected = new Map();
        let catalog = [];
        let current = 0;
        let unlocked = 0;

        const themes = () => $$('input[name="theme"]:checked').map((i) => i.value);
        const loader = () => $('input[name="loader"]:checked')?.value || "";
        const mcVersion = () => mcVersionSelect?.value || "";
        const loaderVersion = () => loaderVersionSelect?.value || "";

        const stepValid = {
            0: () => themes().length > 0,
            1: () => Boolean(loader()),
            2: () => Boolean(mcVersion()),
            3: () => selected.size > 0,
            4: () => true,
        };

        function show(n) {
            current = Math.max(0, Math.min(stages.length - 1, n));
            stages.forEach((x, i) => x.classList.toggle("hidden", i !== current));
            steps.forEach((x, i) => {
                x.classList.toggle("active", i === current);
                x.classList.toggle("done", i < current && stepValid[i]());
            });

            prev.disabled = current === 0;
            next.classList.toggle("hidden", current === stages.length - 1);

            const valid = stepValid[current]();
            next.disabled = !valid;
            next.title = valid ? "" : "Заполните этот шаг, чтобы продолжить";

            if (current === 3) {
                refreshSelectionState();
                loadCatalog();
            }
            if (current === 4) renderExportHints();
        }

        function refreshButtons() { show(current); }

        const themeLabels = {
            optimization: "Оптимизация",
            visual: "Визуал",
            "quality-of-life": "Удобство",
        };

        function renderModCard(mod, opts = {}) {
            const isSelected = selected.has(mod.id);

            const label = document.createElement("label");
            label.className = "mod-item";
            label.dataset.modId = mod.id;
            if (isSelected) label.classList.add("mod-selected");

            const box = document.createElement("input");
            box.type = "checkbox";
            box.checked = isSelected;

            const icon = document.createElement("img");
            icon.className = "mod-icon";
            icon.loading = "lazy";
            icon.alt = "";
            if (mod.icon_url) icon.src = mod.icon_url;

            const text = document.createElement("div");
            text.className = "mod-text";

            const title = document.createElement("h3");
            title.textContent = mod.title || mod.slug || mod.id;

            const meta = document.createElement("p");
            meta.className = "muted";
            const parts = [];
            if (opts.showAuthor && mod.author) parts.push(mod.author);
            if (mod.description) parts.push(mod.description);
            meta.textContent = parts.join(" — ");

            text.append(title, meta);

            if (Array.isArray(mod.themes) && mod.themes.length) {
                const tags = document.createElement("div");
                tags.className = "mod-tags";
                mod.themes.forEach((t) => {
                    const tag = document.createElement("span");
                    tag.className = "mod-tag";
                    tag.textContent = themeLabels[t] || t;
                    tags.append(tag);
                });
                text.append(tags);
            }

            label.append(icon, box, text);

            label.addEventListener("change", () => {
                if (box.checked) selected.set(mod.id, mod);
                else selected.delete(mod.id);
                refreshSelectionState();
                refreshButtons();
            });

            return label;
        }

        function renderSummary() {
            if (!selectionSummary) return;

            if (selected.size === 0) {
                selectionSummary.textContent = "Моды не выбраны";
                return;
            }

            let text = `Выбрано модов: ${selected.size}`;

            if (catalog.length) {
                const visible = new Set(catalog.map((m) => m.id));
                const outside = [...selected.keys()].filter((id) => !visible.has(id)).length;
                if (outside > 0) {
                    text += ` (${outside} вне фильтра каталога — `
                        + `попадут в сборку, только если поддерживают выбранную версию)`;
                }
            }
            selectionSummary.textContent = text;
        }

        function refreshSelectionState() {
            $$(".mod-item").forEach((el) => {
                const on = selected.has(el.dataset.modId);
                el.classList.toggle("mod-selected", on);
                const box = el.querySelector('input[type="checkbox"]');
                if (box) box.checked = on;
            });
            renderSummary();
        }

        let catalogRequest = 0;

        function applyCatalogFilter() {
            if (!modList) return;
            const query = (catalogSearch?.value || "").trim().toLowerCase();

            const items = query
                ? catalog.filter((m) => `${m.title} ${m.description}`.toLowerCase().includes(query))
                : catalog;

            modList.replaceChildren();
            items.forEach((mod) => modList.append(renderModCard(mod)));
            refreshSelectionState();
        }

        async function loadCatalog() {
            if (!modList || !loader() || !mcVersion()) return;

            const requestId = ++catalogRequest;
            const status = catalogStatus;

            modList.replaceChildren();
            if (status) status.textContent = "Загружаем каталог...";

            try {
                const { listCatalog } = await import("./modpack-client.js");
                const mods = await listCatalog(themes(), loader());
                if (requestId !== catalogRequest) return;

                catalog = mods;
                applyCatalogFilter();

                if (status) {
                    status.textContent = catalog.length
                        ? `В каталоге для выбранных тем и загрузчика: ${catalog.length}`
                        : "Для выбранных тем и загрузчика подходящих модов нет — используйте поиск ниже.";
                }
            } catch (e) {
                if (requestId !== catalogRequest) return;
                catalog = [];
                modList.replaceChildren();
                if (status) status.textContent = e.message;
                console.error("Каталог не загрузился", e);
            }
        }

        catalogSearch?.addEventListener("input", applyCatalogFilter);

        let searchRequest = 0;
        let searchTimer = null;

        async function searchMods() {
            if (!searchResults) return;
            const query = (searchInput?.value || "").trim();
            const searchLoader = loader();
            const searchVersion = mcVersion();
            const requestId = ++searchRequest;
            searchResults.replaceChildren();

            if (!query) {
                if (searchStatus) searchStatus.textContent = "Введите название мода для поиска.";
                return;
            }
            if (!searchLoader || !searchVersion) {
                if (searchStatus) {
                    searchStatus.textContent =
                        "Сначала выберите загрузчик и версию Minecraft.";
                }
                return;
            }

            if (searchStatus) searchStatus.textContent = "Ищем моды на Modrinth...";

            const stale = () => requestId !== searchRequest || loader() !== searchLoader ||
                mcVersion() !== searchVersion || searchInput.value.trim() !== query;
            try {
                const { findMods } = await import("./modpack-client.js");
                const mods = await findMods(query, searchLoader, searchVersion);
                if (stale()) return;
                mods.forEach((mod) => searchResults.append(renderModCard(mod, { showAuthor: true })));
                refreshSelectionState();

                if (searchStatus) {
                    searchStatus.textContent = mods.length
                        ? `Найдено модов: ${mods.length}`
                        : "Ничего не найдено. Попробуйте другой запрос.";
                }
            } catch (e) {
                if (stale()) return;
                if (searchStatus) searchStatus.textContent = e.message;
                console.error("Поиск модов не удался", e);
            }
        }

        searchInput?.addEventListener("input", () => {
            clearTimeout(searchTimer);
            searchTimer = setTimeout(searchMods, 600);
        });
        searchInput?.addEventListener("keydown", (e) => {
            if (e.key === "Enter") {
                e.preventDefault();
                clearTimeout(searchTimer);
                searchMods();
            }
        });
        searchButton?.addEventListener("click", () => {
            clearTimeout(searchTimer);
            searchMods();
        });

        function currentFormat() {
            return $('input[name="export_format"]:checked')?.value || "mods-zip";
        }

        function renderExportHints() {
            if (exportFormatHint) {
                exportFormatHint.textContent = formatHints[currentFormat()] || "";
            }
            if (instructionContent && instructionSelect) {
                instructionContent.textContent =
                    instructionsText[instructionSelect.value] || "";
            }
        }

        $$('input[name="export_format"]').forEach((r) =>
            r.addEventListener("change", renderExportHints));
        instructionSelect?.addEventListener("change", renderExportHints);

        downloadButton?.addEventListener("click", async () => {
            const status = $("#export-status");
            const mods = [...selected.values()];

            if (!mods.length) {
                if (status) status.textContent = "Сначала выберите хотя бы один мод.";
                return;
            }

            downloadButton.disabled = true;
            if (status) status.textContent = "Собираем сборку, это может занять время...";

            const request = {
                loader: loader(),
                mc_version: mcVersion(),
                loader_version: loaderVersion(),
                pack_name: packNameInput?.value || "",
                java_args: javaArgsInput?.value || "",
                instructions: instructionsText[instructionSelect?.value] || "",
                format: currentFormat(),
                mods,
            };
            try {
                const { exportPack } = await import("./modpack-client.js");
                const { blob, filename, skipped, local } = await exportPack(request, () => {
                    if (status) status.textContent = "Собираем архив на сервере...";
                });

                const url = URL.createObjectURL(blob);
                const a = document.createElement("a");
                a.href = url;
                a.download = filename;
                document.body.append(a);
                a.click();
                a.remove();
                setTimeout(() => URL.revokeObjectURL(url), 10000);

                if (status) {
                    const source = local ? "в браузере" : "на сервере";
                    status.textContent = skipped
                        ? `Готово ${source}: ${filename}. Не удалось подобрать версии для: ${skipped}`
                        : `Готово ${source}: ${filename}`;
                }
            } catch (e) {
                if (status) status.textContent = e.message;
                console.error("Экспорт не удался", e);
            } finally {
                downloadButton.disabled = false;
            }
        });

        prev?.addEventListener("click", () => show(current - 1));
        next?.addEventListener("click", () => {
            if (!stepValid[current]()) return;
            unlocked = Math.max(unlocked, current + 1);
            show(current + 1);
        });
        steps.forEach((x) =>
            x.addEventListener("click", () => {
                const target = Number(x.dataset.step);
                if (target <= unlocked) show(target);
            }));

        $$('input[name="theme"], input[name="loader"], #mc-version')
            .forEach((el) => el.addEventListener("change", refreshButtons));

        show(0);
    }

    document.addEventListener("DOMContentLoaded", () => {
        nav();
        wizard();
        cards();
    });
})();
