#include "texture.hpp"
#include "timer.hpp"
#include <iostream>

size_t Texture::get_udim(const float u, const float v)
{
    size_t U = static_cast<size_t>(ceil(u));
    size_t V = static_cast<size_t>(floor(v)) * 10;
    return U + V;
}

Vector2f Texture::localize_uv(const float& u, const float& v)
{
    float u_local = u - floor(u);
    float v_local = v - floor(v);
    Vector2f uv(u_local, v_local);
    return uv;
}

Vector3f Texture::get_pixel_values(const float u, const float v, const std::vector<float>& texture, const int width, const int height, const int nchannel)
{
    // Get pixel values by bilinear filtering

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
void Texture::computeTangentBasis(
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

void Texture::normalDisplacement(Mesh& mesh, std::vector<Image>& imgArray)
{

    std::vector<Vertex> tempVertices;
    tempVertices.resize(mesh.vertices.size());

    int numFaces = static_cast<int>(mesh.faces.size());
#pragma omp parallel for
    for (int i = 0; i < numFaces; i++) {
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

            Vector3f rgb;
            
            if (udim > imgArray.size()) {
                // If the UV is outside the given UDIM range, use the same point
                rgb << 0, 0, 0;
            } else {
                Image& img = imgArray[udim - 1];

                if (img.isEmpty) {
                    // If the UV is within the given UDIM but has no texture, use the same point
                    rgb << 0, 0, 0;
                } else {
                    int width = img.width;
                    int height = img.height;
                    int channels = img.nchannels;
                    Vector2f local_uv = localize_uv(u, v);
                    rgb = get_pixel_values(local_uv.x(), local_uv.y(), img.pixels, width, height, channels);
                }
            }

            Vector3f new_pp = P + (N * rgb.x());
            mesh.vertices[fv.vertexIndex - 1].isDone = true;
            tempVertices[fv.vertexIndex - 1] = new_pp;
        }
    }
    mesh.vertices = tempVertices;
}

void Texture::vectorDisplacement(Mesh& mesh, std::vector<Image>& imgArray)
{
    std::cout << "Setting new point positions..." << std::endl;
    Timer timer;
    timer.start();

    std::vector<Vertex> tempVertices;
    tempVertices.resize(mesh.vertices.size());

    int numFaces = static_cast<int>(mesh.faces.size());
#pragma omp parallel for
    for (int i = 0; i < numFaces; i++) {
        Face& face = mesh.faces[i];
        size_t numFaceVerts = face.FaceVertices.size();

        for (size_t i = 0; i < numFaceVerts; i++) {
            FaceVertex *current, *next, *nextNext;

            if (numFaceVerts - i == 2) {
                // one before the last
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

            Vector3f displace;

            if (udim > imgArray.size()) {
                // If the UV is outside the given UDIM range, use the same point
                displace << 0, 0, 0;
            } else {
                Image& img = imgArray[udim - 1];
                if (img.isEmpty) {
                    // If the UV is within the given UDIM but has no texture, use the same point
                    displace << 0, 0, 0;
                } else {
                    int width = img.width;
                    int height = img.height;
                    int channels = img.nchannels;

                    Vector2f local_uv = localize_uv(u, v);
                    Vector3f rgb;
                    rgb = get_pixel_values(local_uv.x(), local_uv.y(), img.pixels, width, height, channels);
                    displace = rgb.transpose() * mat;
                }
            }

            Vector3f new_pp = P0 + displace;
            mesh.vertices[current->vertexIndex - 1].isDone = true;
            tempVertices[current->vertexIndex - 1] = new_pp;
        }
    }
    mesh.vertices = tempVertices;

    timer.showDuration("Finished displacement : ");
}
