document.addEventListener('DOMContentLoaded', function () {
    const sliders = {
        noiseSize: document.getElementById('noiseSize'),
        offsetX: document.getElementById('offsetX'),
        offsetY: document.getElementById('offsetY'),
        octaves: document.getElementById('octaves'),
        frequency: document.getElementById('frequency'),
        persistence: document.getElementById('persistence')
    };

    let currentTimestamp = Date.now();
    let generateTimeout = null;
    const GENERATE_DELAY = 500;

    Object.keys(sliders).forEach(key => {
        const slider = sliders[key];
        const valueElement = document.getElementById(`${key}Value`);

        slider.addEventListener('input', function () {
            valueElement.textContent = this.value;
            scheduleGenerateNoise(false);
        });
    });

    document.getElementById('generateBtn').addEventListener('click', function () {
        generateNoise(true);
    });

    document.getElementById('saveBtn').addEventListener('click', saveImage);

    generateNoise(true);

    function scheduleGenerateNoise(useNewTimestamp) {
        if (generateTimeout) {
            clearTimeout(generateTimeout);
        }

        generateTimeout = setTimeout(() => {
            generateNoise(useNewTimestamp);
            generateTimeout = null;
        }, GENERATE_DELAY);
    }

    function generateNoise(useNewTimestamp) {
        const imageContainer = document.getElementById('imageContainer');
        const img = new Image(512, 512);
        img.className = "noise-image";
        img.id = "noiseImage";

        if (useNewTimestamp) {
            currentTimestamp = Date.now();
        }

        const params = {
            size: parseInt(sliders.noiseSize.value),
            offsetX: parseInt(sliders.offsetX.value),
            offsetY: parseInt(sliders.offsetY.value),
            octaves: parseInt(sliders.octaves.value),
            frequency: parseFloat(sliders.frequency.value),
            persistence: parseFloat(sliders.persistence.value)
        };

        const noise_url = `/noise/generate?size=${params.size}&offsetX=${params.offsetX}&offsetY=${params.offsetY}&octaves=${params.octaves}&frequency=${params.frequency}&persistence=${params.persistence}&t=${currentTimestamp}`;

        console.log("Generate noise from: ", noise_url);
        img.src = noise_url;

        img.onload = function () {
            imageContainer.innerHTML = '';
            imageContainer.appendChild(img);
        };

        img.onerror = function (event) {
            console.error("Error while load image");
        };
    }

    function saveImage() {
        const image = document.getElementById('noiseImage');
        const link = document.createElement('a');
        link.download = 'noise.bmp';
        //link.href = image.toDataURL('image/bmp');
        link.click();
    }
});