#include "argparse.hpp"
#include "timer.hpp"
#include "utils.hpp"
#include <filesystem>
#include <omp.h>

int main(int argc, char* argv[])
{

    argparse::ArgumentParser program("objModifier", "1.0.0");
    program.add_argument("-o", "--object")
        .required()
        .help("specify the input obj file");

    program.add_argument("-v", "--vector")
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
    } catch (std::logic_error& e) {
        std::cout << "No textures provided" << std::endl;
    }

    std::filesystem::path obj_path = file_in;

    obj_path.replace_filename("out_displaced.obj");
    obj_path.make_preferred();
    std::string out_path = obj_path.string<char>();

    std::filesystem::path tex_path = texture_paths[0];
    std::string texture_ext = tex_path.extension();

    // Load Textures
    std::vector<std::vector<float>> texture_data;
    int width;
    int height;
    int channels;
    texture_data.reserve(texture_paths.size());

    Timer timer;
    timer.start();
    for (auto& path : texture_paths) {

        std::filesystem::path tex_path = texture_paths[0];
        std::string texture_ext = tex_path.extension();

        std::vector<float> imgVec;

        if (texture_ext == ".tif" || texture_ext == ".tiff") {
            loadTiff(path, imgVec, width, height, channels);
        } else if (texture_ext == ".exr") {
            loadExr(path, imgVec, width, height, channels);
        } else {
            std::cout << "textures not supported" << std::endl;
            exit(0);
        }
        texture_data.push_back(imgVec);
    }
    timer.showDuration("Finished loading textures : ");

    std::cout << "Texture size : " << width << "/" << height << std::endl;

    // Load Object
    Mesh obj(file_in);

    // Displacement
    if (program["--vector"] == true) {
        vectorDisplacement(obj, texture_data, width, height, channels);
    } else {
        normalDisplacement(obj, texture_data, width, height, channels);
    }

    // Normal
    obj.setToFacenormal();

    // Write object
    obj.write(out_path);

    return 0;
}
