#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef USE_OPENMP
#include <omp.h>
#endif

#include "objIO.hpp"
#include "timer.hpp"
#include "util.hpp"

Mesh::Mesh() { }

Mesh::Mesh(std::string file_path)
    : path(file_path)
{
    read(file_path);
}

Mesh::~Mesh() { }

void ::Mesh::read(std::string path)
{
    std::cout << "Loading obj..." << std::endl;
    Timer timer;
    timer.start();

    static const char* prefix_v = "v ";
    static const char* prefix_uv = "vt";
    static const char* prefix_nml = "vn";
    static const char* prefix_f = "f ";

    std::vector<std::string> faces_temp;

    const int column = 256;

    char str[column];
    char* ptr;

    FILE* fp;
    fp = fopen(path.c_str(), "r");
    if (fp == NULL) {
        printf("%s file cannot be opened\n", path.c_str());
        exit(EXIT_FAILURE);
    }

    while (fgets(str, column, fp) != NULL) {
        if (strncmp(str, prefix_v, 2) == 0) {
            // Vertex
            ptr = strtok(str, " ");
            int index = 0;
            float xyz[3];

            while (ptr != NULL) {
                ptr = strtok(NULL, " ");
                if (ptr != NULL) {
                    float v = atof(ptr);
                    xyz[index] = v;
                }
                index++;
            }
            Vector3f vec(xyz);
            this->vertices.push_back(vec);
        } else if (strncmp(str, prefix_uv, 2) == 0) {
            // UV
            ptr = strtok(str, " ");
            int index = 0;
            float uvw[3];

            while (ptr != NULL) {
                ptr = strtok(NULL, " ");
                if (ptr != NULL) {
                    float v = atof(ptr);
                    uvw[index] = v;
                }
                index++;
            }
            Vector3f vec(uvw);
            this->texcoods.push_back(vec);
        } else if (strncmp(str, prefix_nml, 2) == 0) {
            // Normal
            ptr = strtok(str, " ");
            int index = 0;
            float xyz[3];

            while (ptr != NULL) {
                ptr = strtok(NULL, " ");
                if (ptr != NULL) {
                    float v = atof(ptr);
                    xyz[index] = v;
                }
                index++;
            }
            Vector3f vec(xyz);
            this->normals.push_back(vec);
        } else if (strncmp(str, prefix_f, 2) == 0) {
            // Face
            std::string f(str);
            f.erase(std::remove(f.begin(), f.end(), '\n'), f.end());
            faces_temp.push_back(f);
        }
    }

    fclose(fp);

    std::vector<std::string> strs;
    std::vector<std::string> indices;

    for (auto& face_str : faces_temp) {
        Utils::split(face_str, strs, ' ');

        Face f;

        for (size_t i = 1; i < strs.size(); i++) {
            Utils::split(strs[i], indices, '/');

            FaceVertex fv;

            fv.vertexIndex = std::stoi(indices[0]);
            fv.uvIndex = std::stoi(indices[1]);

            // When normals are soften, obj has normals per vertex so vert index = normal index
            fv.normalIndex = std::stoi(indices[0]);

            if (indices.size() == 3) {
                fv.normalIndex = std::stoi(indices[2]);
            }

            f.FaceVertices.push_back(fv);
        }
        this->faces.push_back(f);
    }

    timer.showDuration("Obj loaded : ");
    std::cout << "Finished loading geometry" << std::endl;

    std::cout << this->vertices.size() << " verts" << std::endl;
    std::cout << this->texcoods.size() << " UVs" << std::endl;
    std::cout << this->normals.size() << " normals" << std::endl;
    std::cout << this->faces.size() << " faces" << std::endl;

    if (this->normals.size() == 0) {
        std::cout << "Obj file doesn't have normal information. Generating new normals..." << std::endl;
        setToFacenormal();
    }
}

void Mesh::setToFacenormal()
{
    std::cout << "Re-calculating normals" << std::endl;
    Timer timer;
    timer.start();

    std::vector<Vector3f> tempNormals;
    tempNormals.resize(this->vertices.size());

    Vector3f zeroVec(0, 0, 0);
    std::fill(tempNormals.begin(), tempNormals.end(), zeroVec);

    int numFaces = static_cast<int>(this->faces.size());
#pragma omp parallel for
    for (int i = 0; i < numFaces; i++) {
        Face& face = this->faces[i];
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

            Vector3f P0 = this->vertices[current->vertexIndex - 1];
            Vector3f P1 = this->vertices[next->vertexIndex - 1];
            Vector3f P2 = this->vertices[nextNext->vertexIndex - 1];

            // Recalculate normals
            Vector3f E1 = P1 - P0;
            Vector3f E2 = P2 - P0;
            Vector3f faceNormal = E1.cross(E2);
            tempNormals[current->normalIndex - 1] += faceNormal;
        }
    }

    for (auto& N : tempNormals) {
        N.normalize();
    }
    this->normals = tempNormals;

    timer.showDuration("Normal recalculated : ");
}

void Mesh::write(std::string out_path)
{
    Timer timer;
    timer.start();

    std::cout << "Writing new obj file..." << std::endl;

    FILE* fp;
    fp = fopen(out_path.c_str(), "w");

    if (fp == NULL) {
        printf("%s file cannot be opened\n", out_path.c_str());
        exit(EXIT_FAILURE);
    }

    char line[128];

    for (Vector3f& v : this->vertices) {
        line[0] = '\0'; // clear
        sprintf(line, "v %f %f %f\n", v.x(), v.y(), v.z());
        fputs(line, fp);
    }

    for (Vector3f& vt : this->texcoods) {
        line[0] = '\0'; // clear
        sprintf(line, "vt %f %f %f\n", vt.x(), vt.y(), vt.z());
        fputs(line, fp);
    }

    for (Vector3f& vn : this->normals) {
        line[0] = '\0'; // clear
        sprintf(line, "vn %f %f %f\n", vn.x(), vn.y(), vn.z());
        fputs(line, fp);
    }

    char indices[32];

    for (Face& face : this->faces) {
        line[0] = '\0'; // clear
        sprintf(line, "f");
        for (FaceVertex& fv : face.FaceVertices) {
            indices[0] = '\0'; // clear
            sprintf(indices, " %lu/%lu/%lu", fv.vertexIndex, fv.uvIndex, fv.normalIndex);
            strcat(line, indices);
        }
        strcat(line, "\n");
        fputs(line, fp);
    }

    fclose(fp);

    timer.showDuration("Output obj exported : ");
}
