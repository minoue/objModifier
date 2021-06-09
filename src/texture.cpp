#define TINYEXR_IMPLEMENTATION

#include "texture.hpp"
#include "tiffio.hxx"
#include "tinyexr.h"
#include "util.hpp"

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

void Image::loadTif(const std::string& path)
{
    // int& width = this->width;
    // int& height = this->height;
    // int& nchannels = this->nchannels;

    std::cout << "Loading tif :" << path << std::endl;

    TIFF* tif = TIFFOpen(path.c_str(), "r");
    if (tif) {
        tdata_t buf;
        uint16_t nchannels;

        TIFFGetField(tif, TIFFTAG_IMAGELENGTH, this->height);
        TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, this->width);
        TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, this->nchannels);

        nchannels = static_cast<int>(this->nchannels);

        this->pixels.reserve(width * height * nchannels);

        buf = _TIFFmalloc(TIFFScanlineSize(tif));
        for (int row = 0; row < height; row++) {
            TIFFReadScanline(tif, buf, row);
            for (int col = 0; col < width; col++) {
                // uint16_t r = static_cast<uint16_t*>(buf)[col * nchannels + 0];
                // uint16_t g = static_cast<uint16_t*>(buf)[col * nchannels + 1];
                // uint16_t b = static_cast<uint16_t*>(buf)[col * nchannels + 2];

                float r = static_cast<float*>(buf)[col * nchannels + 0];
                float g = static_cast<float*>(buf)[col * nchannels + 1];
                float b = static_cast<float*>(buf)[col * nchannels + 2];

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
