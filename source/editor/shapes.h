#pragma once
#include <filesystem>
#include <vector>

#include "glfw_utils.h"

namespace NEONnoir
{
    struct shape
    {
        uint16_t x{ 0 }, y{ 0 };
        uint16_t width{ 0 }, height{ 0 };
    };

    struct shape_container
    {
        std::string image_file;
        std::vector<shape> shapes;
        MPG::simple_image image;
        GLtexture texture;
    };

    std::vector<shape_container> load_shape_json(std::filesystem::path const& file_path);

    void save_shape_json(std::filesystem::path const& file_path, std::vector<shape_container> const& shapes);
    void save_shape_mpsh(std::filesystem::path const& file_path, std::vector<shape_container> const& shapes, uint8_t bit_depth);
    void save_shape_blitz(std::filesystem::path const& file_path, std::vector<shape_container> const& shapes, uint8_t bit_depth);
}