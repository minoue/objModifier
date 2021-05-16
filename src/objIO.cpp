#include "objIO.hpp"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <cstddef>
#include <cstring>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "timer.hpp"

namespace {
size_t split(const std::string& txt, std::vector<std::string>& strs, char ch)
{
    size_t pos = txt.find(ch);
    size_t initialPos = 0;
    strs.clear();

    // Decompose statement
    while (pos != std::string::npos) {
        strs.push_back(txt.substr(initialPos, pos - initialPos));
        initialPos = pos + 1;

        pos = txt.find(ch, initialPos);
    }

    // Add the last one
    strs.push_back(txt.substr(initialPos, std::min(pos, txt.size()) - initialPos + 1));

    return strs.size();
}

// https://www.oreilly.com/library/view/c-cookbook/0596007612/ch03s06.html
// float sciToFloat(const std::string& str) {
//
//     std::stringstream ss(str);
//     float d = 0;
//     ss >> d;
//
//     if (ss.fail()) {
//         std::string s = "Unable to format ";
//         s += str;
//         s += " as a number!";
//         throw (s);
//     }
//
// return (d);
// }
}

Mesh::Mesh() { }

Mesh::Mesh(std::string file_path)
    : path(file_path)
{
    // read(file_path);
    read2(file_path);
}

Mesh::~Mesh() { }

void ::Mesh::read(std::string path)
{
    std::cout << "Loading obj..." << std::endl;
    Timer timer;
    timer.start();

    std::ifstream file_in;
    file_in.open(path, std::ios::in);

    if (!file_in) {
        std::cout << "can't open the file" << std::endl;
        exit(1);
    }

    std::vector<std::string> all_lines;
    std::string tempLine;

    while (!file_in.eof()) {
        std::getline(file_in, tempLine);
        all_lines.push_back(tempLine);
    }

    file_in.close();

    for (size_t i = 0; i < all_lines.size(); i++) {
        std::string& line = all_lines[i];

        std::vector<std::string> words;
        split(line, words, ' ');
        std::string& prefix = words[0];

        size_t wordCount = words.size();
        float x, y, z;

        if (prefix == "v") {
            x = static_cast<float>(atof(words[1].c_str()));
            y = static_cast<float>(atof(words[2].c_str()));
            z = static_cast<float>(atof(words[3].c_str()));
            Vertex v(x, y, z);
            this->vertices.push_back(v);
        } else if (prefix == "vt") {
            x = static_cast<float>(atof(words[1].c_str()));
            y = static_cast<float>(atof(words[2].c_str()));
            if (wordCount == 3) {
                z = 0.0f;
            } else {
                z = static_cast<float>(atof(words[3].c_str()));
            }
            Vector3f v(x, y, z);
            this->texcoods.push_back(v);
        } else if (prefix == "vn") {
            x = static_cast<float>(atof(words[1].c_str()));
            y = static_cast<float>(atof(words[2].c_str()));
            z = static_cast<float>(atof(words[3].c_str()));
            Vector3f v(x, y, z);
            this->normals.push_back(v);
        } else if (prefix == "f") {
            Face f;
            for (size_t i = 1; i < words.size(); i++) {
                std::string& vertexInfo = words[i];
                std::vector<std::string> indecies;
                split(vertexInfo, indecies, '/');

                FaceVertex fv;

                fv.vertexIndex = std::stoi(indecies[0]);
                fv.uvIndex = std::stoi(indecies[1]);
                
                // When normals are soften, obj has normals per vertex so vert index = normal index
                fv.normalIndex = std::stoi(indecies[0]);

                if (indecies.size() == 3) {
                    fv.normalIndex = std::stoi(indecies[2]);
                }

                f.FaceVertices.push_back(fv);
            }
            this->faces.push_back(f);
        } else {
        }
    }

    timer.showDuration("Obj loaded : ");

    std::cout << "Finished loading geometry" << std::endl;

    std::cout << this->vertices.size() << " verts" << std::endl;
    std::cout << this->texcoods.size() << " UVs" << std::endl;
    std::cout << this->normals.size() << " normals" << std::endl;
    std::cout << this->faces.size() << " faces" << std::endl;

    this->vertices.resize(vertices.size());

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

#pragma omp parallel for
    for (size_t i = 0; i < this->faces.size(); i++) {
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

void ::Mesh::write(std::string out_path)
{
    Timer timer;
    timer.start();

    std::cout << "Writing new obj file" << std::endl;

    std::ofstream output_file(out_path);
    for (Vector3f& v : this->vertices) {
        output_file << "v " << std::to_string(v.x()) << " " << std::to_string(v.y()) << " " << std::to_string(v.z()) << "\n";
    }
    for (Vector3f& vt : this->texcoods) {
        output_file << "vt " << std::to_string(vt.x()) << " " << std::to_string(vt.y()) << " " << std::to_string(vt.z()) << "\n";
    }
    for (Vector3f& vn : this->normals) {
        output_file << "vn " << std::to_string(vn.x()) << " " << std::to_string(vn.y()) << " " << std::to_string(vn.z()) << "\n";
    }
    for (Face& face : this->faces) {
        output_file << "f ";
        for (FaceVertex& fv : face.FaceVertices) {
            output_file << std::to_string(fv.vertexIndex) << "/" << std::to_string(fv.uvIndex) << "/" << std::to_string(fv.normalIndex) << " ";
        }
        output_file << "\n";
    }
    output_file.close();

    timer.showDuration("Output obj exported : ");
}

void ::Mesh::read2(std::string path) {
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
    
    FILE *fp;
    fp = fopen(path.c_str(), "r");
    if (fp == NULL) {
        printf("%s file cannot be opened\n", path.c_str());
        exit(1);
    }

    while(fgets(str, column, fp) != NULL) {
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
        split(face_str, strs, ' ');

        Face f;

        for (size_t i=1; i<strs.size(); i++) {
            split(strs[i], indices, '/');

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
