#pragma once

#include "timer.hpp"
#include <iostream>
#include <vector>

class Image {
public:
    Image();
    Image(std::string path);
    ~Image();
    int width;
    int height;
    int nchannels;
    std::vector<float> pixels;
    void read(const std::string path);

private:
    void loadExr(const std::string& path);
    void loadTif(const std::string& path);
};
