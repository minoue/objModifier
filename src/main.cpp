#include "argparse.hpp"
#include "texture.hpp"
#include "texture2.hpp"
#include "timer.hpp"
#include "util.hpp"
#include <string>

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
        exit(EXIT_FAILURE);
    }

    std::string file_in = program.get<std::string>("--object");
    std::vector<std::string> texture_paths;

    try {
        texture_paths = program.get<std::vector<std::string>>("textures");
        std::cout << texture_paths.size() << " textures provided" << std::endl;
    } catch (std::logic_error& e) {
        std::cout << "No textures provided" << std::endl;
    }

    std::string out_path = Utils::pathReplaceBody(file_in, "out_displaced");

    // Load Textures
    std::vector<Image> texture_data;
    texture_data.reserve(texture_paths.size());

    int max_udim = 0;
    for (auto& path : texture_paths) {
        std::string texture_udim = Utils::pathGetUdim(path);
        int udim = std::stoi(texture_udim) - 1000;
        if (udim > max_udim) {
            max_udim = udim;
        }
    }
    texture_data.resize(max_udim);

    Timer timer;
    timer.start();
    for (auto& path : texture_paths) {

        Image img(path);

        std::string texture_udim = Utils::pathGetUdim(path);
        int udim = std::stoi(texture_udim) - 1000;
        texture_data[udim - 1] = img;
    }
    timer.showDuration("Finished loading textures : ");

    // Load Object
    Mesh obj(file_in);

    // Displacement
    if (program["--vector"] == true) {
        Texture::vectorDisplacement(obj, texture_data);
    } else {
        Texture::normalDisplacement(obj, texture_data);
    }

    // Normal
    obj.setToFacenormal();

    // Write object
    obj.write(out_path);

    return 0;
}
