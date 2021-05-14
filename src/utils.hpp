#pragma once

#define TINYEXR_IMPLEMENTATION

#include <iostream>

#include "objIO.hpp"
#include "tiffio.hxx"
#include "tinyexr.h"

void loadExr(const std::string& path, std::vector<float>& outVector,
    int& width,
    int& height,
    int& channels)
{

    channels = 4;

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
        int size = width * height * channels;
        outVector.resize(size);
        for (int i = 0; i < size; i++) {
            float x = out[i];
            outVector[i] = x;
        }

        free(out); // release memory of image data
    }
}

void loadTiff(std::string& path, std::vector<float>& out, int& width, int& height, int& channels)
{
    std::cout << "Loading tif :" << path << std::endl;

    TIFF* tif = TIFFOpen(path.c_str(), "r");
    if (tif) {
        tdata_t buf;
        uint16_t nchannels;

        TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);
        TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
        TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &nchannels);

        channels = static_cast<int>(nchannels);

        out.reserve(width * height * nchannels);

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

                out.push_back(r);
                out.push_back(g);
                out.push_back(b);
            }
        }
        _TIFFfree(buf);
        TIFFClose(tif);
    } else {
        std::cout << "err" << std::endl;
        exit(0);
    }
}

size_t get_udim(const float u, const float v)
{
    size_t U = static_cast<size_t>(ceil(u));
    size_t V = static_cast<size_t>(floor(v)) * 10;
    return U + V;
}

Vector2f localize_uv(const float& u, const float& v)
{
    float u_local = u - floor(u);
    float v_local = v - floor(v);
    Vector2f uv(u_local, v_local);
    return uv;
}

Vector3f get_pixel_values(const float u, const float v, const std::vector<float>& texture, const int width, const int height, const int nchannel)
{
    // Get pixel values by bilinear filtering
    //
    float float_width = static_cast<float>(width);
    float float_height = static_cast<float>(height);

    int x1 = static_cast<int>(std::round(float_width * u));
    int x2 = x1 + 1;
    int y1 = static_cast<int>(std::round(float_height * (1 - v)));
    int y2 = y1 + 1;

    size_t target_pixel1 = ((width * (y1 - 1) + x1) - 1) * nchannel;
    size_t target_pixel2 = ((width * (y1 - 1) + x2) - 1) * nchannel;
    size_t target_pixel3 = ((width * (y2 - 1) + x1) - 1) * nchannel;
    size_t target_pixel4 = ((width * (y2 - 1) + x2) - 1) * nchannel;

    Vector3f A;
    A << texture[target_pixel1], texture[target_pixel1 + 1], texture[target_pixel1 + 2];
    Vector3f B;
    B << texture[target_pixel2], texture[target_pixel2 + 1], texture[target_pixel2 + 2];
    Vector3f C;
    C << texture[target_pixel3], texture[target_pixel3 + 1], texture[target_pixel3 + 2];
    Vector3f D;
    D << texture[target_pixel4], texture[target_pixel4 + 1], texture[target_pixel4 + 2];

    float u1 = (static_cast<float>(x1) - 0.5f) / float_width;
    float u2 = (static_cast<float>(x2) - 0.5f) / float_width;
    float v1 = (static_cast<float>(y1) - 0.5f) / float_height;
    float v2 = (static_cast<float>(y2) - 0.5f) / float_height;

    Vector3f E = ((u2 - u) / (u2 - u1)) * A + ((u - u1) / (u2 - u1)) * B;
    Vector3f F = ((u2 - u) / (u2 - u1)) * C + ((u - u1) / (u2 - u1)) * D;
    Vector3f G = ((v2 - (1 - v)) / (v2 - v1)) * E + (((1 - v) - v1) / (v2 - v1)) * F;

    return G;
}

// https://stackoverflow.com/questions/5255806/how-to-calculate-tangent-and-binorma
void computeTangentBasis(
    const Vector3f& A, const Vector3f& B, const Vector3f& C,
    const Vector3f& H, const Vector3f& K, const Vector3f& L,
    Vector3f& T, Vector3f& U, Vector3f& N)
{

    Vector3f D = B - A;
    Vector3f E = C - A;
    Vector3f F = K - H;
    Vector3f G = L - H;

    MatrixXf DE(2, 3);
    DE << D.x(), D.y(), D.z(),
        E.x(), E.y(), E.z();

    Matrix2f FG;
    FG << F.x(), F.y(),
        G.x(), G.y();

    MatrixXf result(2, 3);
    result = FG.inverse() * DE;

    Vector3f new_T = result.row(0);
    new_T -= N * new_T.dot(N);
    new_T.normalize();
    Vector3f bitangent = N.cross(new_T);

    T = new_T;
    U = bitangent.normalized();
}

void normalDisplacement(Mesh& mesh, std::vector<std::vector<float>>& data, int width, int height, int channels)
{

    std::vector<Vertex> tempVertices;
    tempVertices.resize(mesh.vertices.size());

#pragma omp parallel for
    for (size_t i = 0; i < mesh.faces.size(); i++) {
        Face& face = mesh.faces[i];
        size_t numFaceVerts = face.FaceVertices.size();
        for (size_t j = 0; j < numFaceVerts; j++) {
            FaceVertex& fv = face.FaceVertices[j];

            if (mesh.vertices[fv.vertexIndex - 1].isDone) {
                continue;
            }

            Vector3f P = mesh.vertices[fv.vertexIndex - 1];
            Vector3f UV = mesh.texcoods[fv.uvIndex - 1];
            Vector3f N = mesh.normals[fv.normalIndex - 1];
            float u = UV.x();
            float v = UV.y();
            size_t udim = get_udim(u, v);
            Vector2f local_uv = localize_uv(u, v);

            Vector3f rgb;
            rgb = get_pixel_values(local_uv.x(), local_uv.y(), data[udim - 1], width, height, channels);

            Vector3f new_pp = P + (N * rgb.x());
            mesh.vertices[fv.vertexIndex - 1].isDone = true;
            tempVertices[fv.vertexIndex - 1] = new_pp;
        }
    }
    mesh.vertices = tempVertices;
}

void vectorDisplacement(
    Mesh& mesh,
    std::vector<std::vector<float>>& data,
    int width, int height, int channels)
{

    std::vector<Vertex> tempVertices;
    tempVertices.resize(mesh.vertices.size());

#pragma omp parallel for
    for (size_t i = 0; i < mesh.faces.size(); i++) {
        Face& face = mesh.faces[i];
        size_t numFaceVerts = face.FaceVertices.size();

        for (size_t i = 0; i < numFaceVerts; i++) {
            FaceVertex *current, *next, *nextNext;

            if (numFaceVerts - i == 2) {
                //one before the last
                current = &face.FaceVertices[i];
                next = &face.FaceVertices[i + 1];
                nextNext = &face.FaceVertices[0];
            } else if (numFaceVerts - i == 1) {
                // last triangle
                current = &face.FaceVertices[i];
                next = &face.FaceVertices[0];
                nextNext = &face.FaceVertices[1];
            } else {
                current = &face.FaceVertices[i];
                next = &face.FaceVertices[i + 1];
                nextNext = &face.FaceVertices[i + 2];
            }

            if (mesh.vertices[current->vertexIndex - 1].isDone) {
                continue;
            }

            Vector3f P0 = mesh.vertices[current->vertexIndex - 1];
            Vector3f P1 = mesh.vertices[next->vertexIndex - 1];
            Vector3f P2 = mesh.vertices[nextNext->vertexIndex - 1];

            Vector3f uv0 = mesh.texcoods[current->uvIndex - 1];
            Vector3f uv1 = mesh.texcoods[next->uvIndex - 1];
            Vector3f uv2 = mesh.texcoods[nextNext->uvIndex - 1];

            Vector3f T, B, N;
            N = mesh.normals[current->normalIndex - 1];
            N.normalize();

            computeTangentBasis(P0, P1, P2, uv0, uv1, uv2, T, B, N);

            Matrix3f mat;
            mat << T.x(), T.y(), T.z(),
                N.x(), N.y(), N.z(),
                B.x(), B.y(), B.z();

            float u = uv0.x();
            float v = uv0.y();
            size_t udim = get_udim(u, v);

            Vector2f local_uv = localize_uv(u, v);

            Vector3f rgb;
            rgb = get_pixel_values(local_uv.x(), local_uv.y(), data[udim - 1], width, height, channels);

            Vector3f displace;

            displace = rgb.transpose() * mat;

            Vector3f new_pp = P0 + displace;

            mesh.vertices[current->vertexIndex - 1].isDone = true;
            tempVertices[current->vertexIndex - 1] = new_pp;
        }
    }
    mesh.vertices = tempVertices;
}
