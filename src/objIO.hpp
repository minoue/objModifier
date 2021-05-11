#pragma once

#include <string>
#include <vector>

#include "Eigen/Core"
#include "Eigen/LU"
#include "Eigen/Dense"


using namespace Eigen;


class Vertex : public Vector3f {
    public:
        using Vector3f::Matrix;
        bool isDone = false;
};


struct FaceVertex {
    size_t vertexIndex;
    size_t uvIndex;
    size_t normalIndex;
};


struct Face {
    std::vector<FaceVertex> FaceVertices;
    size_t faceIndex;
};

class Mesh {
    public:
        Mesh();
        Mesh(std::string file_path);
        virtual ~Mesh();
        void echoPath();
        void read(std::string in_path);
        void write(std::string out_path);
        std::string path;
        std::vector<Vertex> vertices;
        std::vector<Vector3f> texcoods;
        std::vector<Vector3f> normals;
        std::vector<Face> faces;
        void setToFacenormal();
    private:
};
