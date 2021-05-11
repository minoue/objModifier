#include "argparse.hpp"
#include "timer.hpp"
#include "utils.hpp"
#include <filesystem>
#include <omp.h>

int main(int argc, char* argv[])
{

    argparse::ArgumentParser program("objModifier", "1.0.0");
    program.add_argument("-v", "--verbose")
        .help("Increase output verbosity")
        .default_value(false)
        .implicit_value(true);

    program.add_argument("-obj", "--object")
        .required()
        .help("specify the input obj file");

    program.add_argument("-vec", "--vector")
        .help("Vector displacement")
        .default_value(true)
        .implicit_value(true);

    program.add_argument("textures")
        .remaining();

    try {
        program.parse_args(argc, argv);
    } catch (const std::runtime_error& err) {
        std::cout << err.what() << std::endl;
        std::cout << program;
        exit(0);
    }

    std::string file_in = program.get<std::string>("--object");
    std::vector<std::string> texture_paths;

    try {
        texture_paths = program.get<std::vector<std::string>>("textures");
        std::cout << texture_paths.size() << " textures provided" << std::endl;
        for (auto& tex : texture_paths) {
            std::cout << tex << std::endl;
        }
    } catch (std::logic_error& e) {
        std::cout << "No textures provided" << std::endl;
    }

    if (program["--verbose"] == true) {
        std::cout << "Verbosity enabled" << std::endl;
    }

    // std::string file_in = argv[1];
    // std::vector<std::string> texture_paths(argv+2, argv + argc);

    std::filesystem::path obj_path = file_in;

    obj_path.replace_filename("out_displaced.obj");
    obj_path.make_preferred();
    std::string out_path = obj_path.string<char>();

    std::filesystem::path tex_path = texture_paths[0];
    std::string texture_ext = tex_path.extension();

    Timer timer;

    // Load Textures
    std::vector<std::vector<float>> texture_data;
    int width;
    int height;
    int channels;

    timer.start();
    if (texture_ext == ".tif" || texture_ext == ".tiff") {
        for (auto& path : texture_paths) {
            std::vector<float> imgVec;
            loadTiff(path, imgVec, width, height, channels);
            texture_data.push_back(imgVec);
        }
    } else if (texture_ext == ".exr") {
        loadExr(texture_paths, texture_data, width, height, channels);
    } else {
        std::cout << "textures not supported" << std::endl;
        exit(0);
    }
    timer.showDuration("Texture loaded: ");

    std::cout << "Texture size: " << width << "/" << height << std::endl;
    std::cout << "number of textures loaded: " << texture_data.size() << std::endl;

    // Load Object
    std::cout << "Obj: " << file_in << std::endl;

    timer.start();
    Mesh obj(file_in);
    timer.showDuration("Obj loaded: ");

    // Normals
    std::cout << "Re-calculating normals" << std::endl;
    obj.setToFacenormal();

    // Displacement
    std::cout << "Setting new point positions" << std::endl;
    timer.start();
    if (program["--vector"] == true) {
        vectorDisplacement(obj, texture_data, width, height, channels);
    } else {
        normalDisplacement(obj, texture_data, width, height, channels);
    }
    timer.showDuration("Displacement done: ");

    // Normal
    std::cout << "Re-calculating normals" << std::endl;
    obj.setToFacenormal();

    // Write object
    obj.write(out_path);

    return 0;
}
