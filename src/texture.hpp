#pragma once

#include "image.hpp"
#include "objIO.hpp"
#include <vector>

class Texture {
public:
    Texture();
    ~Texture();
    static void normalDisplacement(Mesh& mesh, std::vector<Image>& imgArray);
    static void vectorDisplacement(Mesh& mesh, std::vector<Image>& imgArray);

private:
    static size_t get_udim(const float u, const float v);
    static Vector2f localize_uv(const float& u, const float& v);
    static Vector3f get_pixel_values(
        const float u,
        const float v,
        const std::vector<float>& texture,
        const int width,
        const int height,
        const int nchannel);
    static void computeTangentBasis(
        const Vector3f& A,
        const Vector3f& B,
        const Vector3f& C,
        const Vector3f& H,
        const Vector3f& K,
        const Vector3f& L,
        Vector3f& T,
        Vector3f& U,
        Vector3f& N);
};
