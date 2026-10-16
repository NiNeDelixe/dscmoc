
const versionSelect = document.getElementById("mc-version");
const versionStatus = document.getElementById("version-status");
let versionRequest = 0;

async function loadVersions() {
    const themes = [...document.querySelectorAll(
        'input[name="theme"]:checked'
    )].map(input => input.value);

    const loader = document.querySelector(
        'input[name="loader"]:checked'
    )?.value;

    const requestId = ++versionRequest;

    versionSelect.replaceChildren(
        new Option("Загрузка версий...", "")
    );
    versionSelect.disabled = true;

    if (!themes.length || !loader) {
        versionSelect.replaceChildren(
            new Option("Выберите тематики и загрузчик модов.", "")
        );
        versionStatus.textContent =
            "Выберите тематики и загрузчик модов.";
        return;
    }

    try {
        const response = await fetch("/mctools/api/modpack/versions/mc", {
            method: "POST",
            headers: { "Content-Type": "application/json" },
            body: JSON.stringify({ themes, loader })
        });

        const data = await response.json();
        if (requestId !== versionRequest) return;
        if (!response.ok) throw new Error("Ошибка получения версий");

        versionSelect.replaceChildren(
            new Option("Выберите версию Minecraft", "")
        );

        data.versions.forEach(version => {
            versionSelect.add(new Option(version.id, version.id));
        });

        versionSelect.disabled = data.versions.length === 0;
        versionStatus.textContent =
            `Найдено версий: ${data.versions.length}`;
    } catch (error) {
        if (requestId !== versionRequest) return;
        versionSelect.replaceChildren(
            new Option("Не удалось загрузить версии", error.message)
        );
        versionStatus.textContent = error.message;
    }
}

document.querySelectorAll(
    'input[name="theme"], input[name="loader"]'
).forEach(input => input.addEventListener("change", loadVersions));



const loaderVersionSelect = document.getElementById("loader-version");
const loaderVersionStatus = document.getElementById("loader-version-status");
const stableOnly = document.getElementById("stable-only");
let loaderRequestId = 0;

async function loadLoaderVersions() {
    const loader = document.querySelector(
        'input[name="loader"]:checked'
    )?.value;
    const mcVersion = document.getElementById("mc-version").value;
    const requestId = ++loaderRequestId;

    loaderVersionSelect.replaceChildren(
        new Option("Загрузка версий...", "")
    );
    loaderVersionSelect.disabled = true;

    if (!loader || !mcVersion) return;

    try {
        const response = await fetch("/mctools/api/modpack/versions/loaders", {
            method: "POST",
            headers: { "Content-Type": "application/json" },
            body: JSON.stringify({ loader, mc_version: mcVersion })
        });

        const data = await response.json();
        if (requestId !== loaderRequestId) return;
        if (!response.ok) throw new Error("Не удалось загрузить версии");

        const versions = (data.versions || []).filter(
            v => !stableOnly.checked || v.is_stable
        );

        loaderVersionSelect.replaceChildren(
            new Option("Выберите версию загрузчика", "")
        );

        versions.forEach(v => {
            loaderVersionSelect.add(new Option(
                `${v.version}${v.is_stable ? " (stable)" : ""}`,
                v.version
            ));
        });

        loaderVersionSelect.disabled = versions.length === 0;
        loaderVersionStatus.textContent =
            `Найдено версий: ${versions.length}`;
    } catch (error) {
        if (requestId !== loaderRequestId) return;
        loaderVersionSelect.replaceChildren(
            new Option("Ошибка загрузки", "")
        );
        loaderVersionStatus.textContent = error.message;
    }
}

document.getElementById("mc-version")
    .addEventListener("change", loadLoaderVersions);

document.querySelectorAll('input[name="loader"]')
    .forEach(input => input.addEventListener("change", loadLoaderVersions));

stableOnly.addEventListener("change", loadLoaderVersions);