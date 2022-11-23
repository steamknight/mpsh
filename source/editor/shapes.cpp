#include <json.hpp>
#include <format>
#include <fstream>
#include <sstream>

#include "utils.h"
#include "shapes.h"

using json = nlohmann::json;
namespace fs = std::filesystem;

namespace NEONnoir
{
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(
        shape,
        x, y,
        width, height
    );

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(
        shape_container,
        image_file,
        shapes
    );

    std::vector<shape_container> load_shape_json(std::filesystem::path const& file_path)
    {
        if (!fs::exists(file_path))
        {
            throw std::runtime_error{ std::format("File '{}' does not exist.", file_path.string()) };
        }

        auto savefile = std::ifstream{ file_path };
        if (savefile)
        {
            auto buffer = std::stringstream{};
            buffer << savefile.rdbuf();

            auto j = json::parse(buffer.str());
            auto containers = j.get<std::vector<shape_container>>();

            for (auto& container : containers)
            {
                container.image = MPG::load_image(container.image_file);
                container.texture = load_texture(container.image);
            }

            return containers;
        }

        throw std::runtime_error{ "Could not read file" };
    }

    void save_shape_json(std::filesystem::path const& file_path, std::vector<shape_container> const& shapes)
    {
        auto savefile = std::ofstream{ file_path, std::ios::trunc };
        if (savefile)
        {
            auto root = json(shapes);
            savefile << root.dump(2);
        }
    }

    void save_shape_mpsh(std::filesystem::path const& file_path, std::vector<shape_container> const& shapes, uint8_t bit_depth)
    {
        auto all_shapes = std::vector<MPG::blitz_shapes>{};

        for (auto const& container : shapes)
        {
            auto  image = MPG::crop_palette(container.image, bit_depth, 0);
            for (auto const& shape : container.shapes)
            {
                auto cropped = MPG::crop(image, shape.x, shape.y, shape.width, shape.height);
                all_shapes.push_back(MPG::image_to_blitz_shapes(cropped));
            }
        }

        if (auto impish_file = std::ofstream{ file_path, std::ios::binary | std::ios::trunc })
        {
            auto offset = 0u;

            // Write header
            char magic[] = { 'M', 'P', 'S', 'H' };
            impish_file.write(magic, 4);                            // Magic number
            write(impish_file, 1u);                                 // Version. Always 1 for now
            write(impish_file, to<uint32_t>(all_shapes.size()));      // Number of shapes
            offset += (sizeof(uint32_t) * 3);

            // Push the offset past where the manifest will go
            // The manifest is 2 uint32_ts (offset and size) per entry
            offset += (sizeof(uint32_t) * 2 * to<uint32_t>(all_shapes.size()));

            // Write the manifest
            for (auto const& shape : all_shapes)
            {
                write(impish_file, offset);

                auto size = to<uint32_t>(shape.get_size());
                write(impish_file, size);

                offset += size;
            }

            // Write all the shapes
            for (auto const& shape : all_shapes)
            {
                // Write the shape header
                write(impish_file, shape.width);
                write(impish_file, shape.height);
                write(impish_file, shape.bit_depth);
                write(impish_file, shape.ebwidth);
                write(impish_file, shape.blitsize);

                // Handle is in the top left. Perhaps I can add support for moving them later
                write(impish_file, shape.handle_x);     // x
                write(impish_file, shape.handle_y);     // y

                // Data and cookie pointers. They seem to always be nonsense values in the shapes files created by Blitz
                write(impish_file, shape.data_ptr);     // data
                write(impish_file, shape.cookie_ptr);     // cookie

                write(impish_file, shape.onebpmem);
                write(impish_file, shape.onebpmemx);
                write(impish_file, shape.allbpmem);
                write(impish_file, shape.allbpmemx);

                write(impish_file, shape.padding);     // padding

                // Write out the shape's bitplanes
                if (shape.data.size() > 0)
                {
                    impish_file.write(force_to<char const*>(&shape.data[0]), shape.data.size());
                }
            }
        }
    }

    void save_shape_blitz(std::filesystem::path const& file_path, std::vector<shape_container> const& shapes, uint8_t bit_depth)
    {
        auto all_shapes = std::vector<MPG::simple_image>{};

        for (auto const& container : shapes)
        {
            auto  image = MPG::crop_palette(container.image, bit_depth, 0);
            for (auto const& shape : container.shapes)
            {
                auto cropped = MPG::crop(image, shape.x, shape.y, shape.width, shape.height);
                all_shapes.push_back(cropped);
            }
        }

        MPG::save_blitz_shapes(file_path, all_shapes);
    }
}