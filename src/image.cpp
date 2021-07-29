#define TINYEXR_IMPLEMENTATION

#include "image.hpp"
#include "tinyexr.h"
#include "util.hpp"

#ifdef USE_TIFF
#include "tiffio.hxx"
constexpr bool kUseTiff = true;
#else
constexpr bool kUseTiff = false;
#endif

Image::Image() {};

Image::Image(std::string path)
{
    read(path);
}

Image::~Image() {};

void Image::read(const std::string path)
{

    std::string ext = Utils::pathGetExt(path);
    if (ext == "exr") {
        loadExr(path);
    } else if (ext == "tif" || ext == "tiff") {
        if (kUseTiff) {
            loadTif(path);
        } else {
            std::cout << "Tiff loader is disabled. Use exr image or use -DUSE_TIFF=1 in CMake build." << std::endl;
            exit(0);
        }
    } else {
        std::cout << "Not supported images" << std::endl;
    }
}

void Image::loadExr(const std::string& path)
{
    int& width = this->width;
    int& height = this->height;
    int& nchannels = this->nchannels;
    nchannels = 4;

    std::cout << "Loading exr :" << path << std::endl;
    float* out;
    const char* err = nullptr;

    int ret = LoadEXR(&out, &width, &height, path.c_str(), &err);
    if (ret != TINYEXR_SUCCESS) {
        if (err) {
            fprintf(stderr, "ERR : %s\n", err);
            FreeEXRErrorMessage(err); // release memory of error message.
            exit(0);
        }
    } else {
        int size = width * height * nchannels;
        this->pixels.resize(size);
        for (int i = 0; i < size; i++) {
            float x = out[i];
            this->pixels[i] = x;
        }

        free(out); // release memory of image data
    }
}

#ifdef USE_TIFF
void Image::loadTif(const std::string& path)
{
    uint32_t width;
    uint32_t height;
    uint16_t nc;

    std::cout << "Loading tif :" << path << std::endl;

    TIFF* tif = TIFFOpen(path.c_str(), "r");
    if (tif) {
        tdata_t buf;

        TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);
        TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
        TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &nc);

        this->nchannels = static_cast<int>(nc);
        this->width = static_cast<int>(width);
        this->height = static_cast<int>(height);

        this->pixels.reserve(width * height * nc);

        buf = _TIFFmalloc(TIFFScanlineSize(tif));
        for (uint32_t row = 0; row < height; row++) {
            TIFFReadScanline(tif, buf, row);
            for (uint32_t col = 0; col < width; col++) {
                // uint16_t r = static_cast<uint16_t*>(buf)[col * nchannels + 0];
                // uint16_t g = static_cast<uint16_t*>(buf)[col * nchannels + 1];
                // uint16_t b = static_cast<uint16_t*>(buf)[col * nchannels + 2];

                float r = static_cast<float*>(buf)[col * nc + 0];
                float g = static_cast<float*>(buf)[col * nc + 1];
                float b = static_cast<float*>(buf)[col * nc + 2];

                this->pixels.push_back(r);
                this->pixels.push_back(g);
                this->pixels.push_back(b);
            }
        }
        _TIFFfree(buf);
        TIFFClose(tif);
    } else {
        std::cout << "err" << std::endl;
        exit(0);
    }
}
#endif
