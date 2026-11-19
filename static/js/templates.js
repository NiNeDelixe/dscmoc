import { minecraftVersions, loaderVersions } from "./modpack-client.js";

(() => {
    const $ = (sel, root = document) => root.querySelector(sel);
    const $$ = (sel, root = document) => [...root.querySelectorAll(sel)];

    const versionSelect = $("#mc-version");
    const loaderVersionSelect = $("#loader-version");

    if (!versionSelect || !loaderVersionSelect) return;

    const versionStatus = $("#version-status");
    const loaderVersionStatus = $("#loader-version-status");
    const stableOnly = $("#stable-only");

    const themes = () => $$('input[name="theme"]:checked').map((i) => i.value);
    const loader = () => $('input[name="loader"]:checked')?.value || "";

    const setStatus = (node, text) => {
        if (node) node.textContent = text;
    };

    let versionRequest = 0;

    async function loadVersions() {
        const requestId = ++versionRequest;

        versionSelect.replaceChildren(new Option("Загрузка версий...", ""));
        versionSelect.disabled = true;
        loadLoaderVersions();

        if (!themes().length || !loader()) {
            versionSelect.replaceChildren(
                new Option("Выберите тематики и загрузчик модов.", ""));
            setStatus(versionStatus, "Выберите тематики и загрузчик модов.");
            return;
        }

        try {
            const versions = await minecraftVersions(loader());
            if (requestId !== versionRequest) return;

            versionSelect.replaceChildren(
                new Option("Выберите версию Minecraft", ""));

            versions.forEach((version) => {
                versionSelect.add(new Option(version.id, version.id));
            });

            versionSelect.disabled = versions.length === 0;
            setStatus(versionStatus, `Найдено версий: ${versions.length}`);
        } catch (error) {
            if (requestId !== versionRequest) return;
            versionSelect.replaceChildren(
                new Option("Не удалось загрузить версии", ""));
            setStatus(versionStatus, error.message);
        }
    }

    $$('input[name="theme"], input[name="loader"]')
        .forEach((input) => input.addEventListener("change", loadVersions));

    let loaderRequestId = 0;

    async function loadLoaderVersions() {
        const currentLoader = loader();
        const mcVersion = versionSelect.value;
        const requestId = ++loaderRequestId;

        loaderVersionSelect.replaceChildren();
        setStatus(loaderVersionStatus, "");

        if (!currentLoader || !mcVersion) {
            loaderVersionSelect.replaceChildren(
                new Option("Сначала выберите версию Minecraft", ""));
            loaderVersionSelect.disabled = true;
            return;
        }

        loaderVersionSelect.replaceChildren(new Option("Загрузка версий...", ""));
        loaderVersionSelect.disabled = true;

        try {
            const available = await loaderVersions(currentLoader, mcVersion);
            if (requestId !== loaderRequestId) return;

            const onlyStable = stableOnly ? stableOnly.checked : false;
            const versions = available.filter((v) => !onlyStable || v.is_stable);

            loaderVersionSelect.replaceChildren(
                new Option("Автоматический выбор", ""));
            versions.forEach((v) => {
                loaderVersionSelect.add(new Option(
                    `${v.version}${v.is_stable ? " (stable)" : ""}`, v.version));
            });

            loaderVersionSelect.disabled = versions.length === 0;
            setStatus(loaderVersionStatus,
                versions.length
                    ? `Найдено версий: ${versions.length}`
                    : "Подходящих версий загрузчика не найдено — сборка соберётся с автоматическим выбором.");
        } catch (error) {
            if (requestId !== loaderRequestId) return;
            loaderVersionSelect.replaceChildren(new Option("Автоматический выбор", ""));
            setStatus(loaderVersionStatus, error.message);
        }
    }

    versionSelect.addEventListener("change", loadLoaderVersions);
    stableOnly?.addEventListener("change", loadLoaderVersions);

    loadVersions();
})();
