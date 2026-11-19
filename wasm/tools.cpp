#include <emscripten/emscripten.h>
#include <minizip/zip.h>

#include "NoiseRender.hpp"

extern "C"
{
    EMSCRIPTEN_KEEPALIVE int write_noise(int size, int x, int y, int octaves,
                                        float frequency, float persistence, int seed)
    {
        const noise_gen::NoiseData data{size, x, y, octaves, frequency, persistence, seed};
        if (!data.valid())
            return 0;
        try
        {
            auto image = noise_gen::NoiseRender(seed, data).createImage();
            noise::utils::WriterBMP writer;
            writer.SetSourceImage(image);
            writer.SetDestFilename("/noise.bmp");
            writer.WriteDestFile();
            return 1;
        }
        catch (...)
        {
            return 0;
        }
    }

    EMSCRIPTEN_KEEPALIVE zipFile open_zip()
    {
        return zipOpen64("/pack.zip", APPEND_STATUS_CREATE);
    }

    EMSCRIPTEN_KEEPALIVE int open_zip_entry(zipFile archive, const char* name)
    {
        // Store entries uncompressed: JARs are already compressed and manifests are small.
        return zipOpenNewFileInZip64(archive, name, nullptr, nullptr, 0, nullptr, 0,
                                    nullptr, 0, 0, 0);
    }
}
