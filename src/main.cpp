#include "CLI11.hh"
#include "texture.hpp"
#include "timer.hpp"
#include "util.hpp"
#include <string>
#include <filesystem>


int main(int argc, char* argv[])
{
    std::vector<std::string> args(argv, argv + argc); 

    CLI::App app{"objModifier"};

    std::vector<std::string> texture_paths;
    std::string input_obj;
    std::string output_obj;
    bool normalDisplacement = false;

    app.add_option("-t, --textures", texture_paths, "Displacement textures");
    app.add_option("-i, --input", input_obj, "Input obj file");
    app.add_option("-o, --output", output_obj, "Output name");
    app.add_flag("-n, --normalDisplacement", normalDisplacement, "Normal displacement");

    CLI11_PARSE(app, argc, argv);

    std::filesystem::path path(input_obj);
    if (output_obj.empty()) {
        path.replace_filename("output.obj");
    } else {
        path.replace_filename(output_obj + ".obj");
    }

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
    Mesh obj(input_obj);

    // Displacement
    if (!normalDisplacement) {
        std::cout << "Applying vector displacement" << std::endl;
        Texture::vectorDisplacement(obj, texture_data);
    } else {
        std::cout << "Applying normal displacement" << std::endl;
        Texture::normalDisplacement(obj, texture_data);
    }

    // Normal
    obj.setToFacenormal();

    // Write object
    obj.write(path.string());

    return 0;
}
