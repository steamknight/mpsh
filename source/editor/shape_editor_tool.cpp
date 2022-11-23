#include "imgui.h"
#include "imgui_stdlib.h"
#include "IconsMaterialDesign.h"
#include "imgui_utils.h"

#include <filesystem>
#include <fstream>
#include <format>

#include "utils.h"
#include "shape_editor_tool.h"

namespace fs = std::filesystem;

namespace NEONnoir
{
    void shape_editor_tool::display()
    {
        auto shapes_editor_window = ImGui_window(ICON_MD_CROP " Shapes Editor Tool", false, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse);

        display_toolbar();

        if (auto table = imgui::table("main_shapes_editor_tool", 2, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_Resizable))
        {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();

            ImGui::SetNextItemWidth(-FLT_MIN);

            if (ImGui::Button(ICON_MD_ADD_PHOTO_ALTERNATE " Add source image", { -FLT_MIN, 0.f }))
            {
                if (auto file = open_file_dialog("iff"))
                {
                    auto container = shape_container{ file.value().data() };
                    
                    // Keep the paths relative
                    container.image_file = fs::relative(file.value(), fs::current_path()).string();
                    container.image = MPG::load_image(file.value());
                    container.texture = load_texture(container.image);

                    _shape_containers.push_back(container);
                }
            }

            auto count = 0u;
            auto shape_id = 0;
            for (auto& container : _shape_containers)
            {
                ImGui::PushID(force_to<void*>(&container));

                if (DeleteButton("##_delete_shape_container"))
                {
                    _shape_container_to_delete = count;
                }

                ImGui::SameLine();
                auto image_name = std::filesystem::path{ container.image_file }.stem().string();
                if (ImGui::Button(image_name.c_str(), { -FLT_MIN, 0.f }))
                {
                    _selected_image = count;
                }

                auto content_width = ImGui::GetColumnWidth() - 16; // leave room for a scroll bar
                auto spacing = ImGui::GetStyle().ItemSpacing.x;
                auto item_width = (content_width - (1 * spacing)) / 2.f;

                if (_selected_image == count)
                {
                    for (auto i = 0; i < _selected_image; i++)
                    {
                        shape_id += to<int32_t>(_shape_containers[i].shapes.size());
                    }

                    auto region_count = 0;
                    //ImGui::BeginChild("regions");
                    for (auto& shape : container.shapes)
                    {
                        ImGui::BeginGroup();

                        ImGui::PushID(force_to<void*>(&shape));

                        ImGui::Text("Shape %d", shape_id + region_count);
                        auto avail = ImGui::GetContentRegionAvail();
                        ImGui::SameLine(avail.x - ImGui::CalcTextSize(ICON_MD_DELETE).x - (1 * spacing));
                        if (DeleteButton("##_delete_shape"))
                        {
                            _shape_to_delete = region_count;
                        }

                        uint16_t const step_size = 1;
                        ImGui::SetNextItemWidth(item_width);
                        ImGui::InputScalar("##_x", ImGuiDataType_U16, &shape.x, &step_size, nullptr, "%u");
                        ImGui::SameLine();
                        ImGui::SetNextItemWidth(item_width);
                        ImGui::InputScalar("##_y", ImGuiDataType_U16, &shape.y, &step_size, nullptr, "%u");
                        //                    ImGui::SameLine();
                        ImGui::SetNextItemWidth(item_width);
                        ImGui::InputScalar("##_width", ImGuiDataType_U16, &shape.width, &step_size, nullptr, "%u");
                        ImGui::SameLine();
                        ImGui::SetNextItemWidth(item_width);
                        ImGui::InputScalar("##_height", ImGuiDataType_U16, &shape.height, &step_size, nullptr, "%u");

                        ImGui::EndGroup();

                        if (ImGui::IsItemHovered())
                        {
                            _shape_image.selected_region(region_count);
                        }

                        ImGui::PopID();

                        region_count++;
                    }
                    //ImGui::EndChild();
                }
                count++;

                ImGui::PopID();
            }

            if (_shape_container_to_delete)
            {
                _shape_containers.erase(_shape_containers.begin() + _shape_container_to_delete.value());
                _shape_container_to_delete = std::nullopt;
                _selected_image = std::nullopt;
            }

            if (_shape_to_delete && _selected_image)
            {
                auto& container = _shape_containers[_selected_image.value()].shapes;
                container.erase(container.begin() + _shape_to_delete.value());

                _shape_to_delete = std::nullopt;
            }

            ImGui::TableNextColumn();
            if (_selected_image.has_value())
            {
                auto& texture = _shape_containers[_selected_image.value()].texture;
                _shape_image.display(texture, _shape_containers[_selected_image.value()].shapes);
            }
        }
    }

    void shape_editor_tool::display_toolbar()
    {
        ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_WindowBg));

        if (ImGui::Button(ICON_MD_FILE_OPEN))
        {
            auto filename = open_file_dialog("json");
            if (filename)
            {
                _shape_containers = load_shape_json(filename.value());
            }
        }
        ToolTip("Load Shapes JSON");
        ImGui::SameLine();

        if (ImGui::Button(ICON_MD_SAVE))
        {
            auto filename = save_file_dialog("json");
            if (filename)
            {
                save_shape_json(filename.value(), _shape_containers);
            }
        }
        ToolTip("Save Shapes JSON");
        ImGui::SameLine();

        if (ImGui::Button(ICON_MD_SWITCH_ACCOUNT))
        {
            auto filename = save_file_dialog("mpsh");
            if (filename)
            {
                save_shape_mpsh(filename.value(), _shape_containers, to<uint8_t>(_export_bit_depth));
            }
        }
        ToolTip("Export MPSH Shapes");
        ImGui::SameLine();

        if (ImGui::Button(ICON_MD_FILE_DOWNLOAD))
        {
            auto filename = save_file_dialog("mpsh");
            if (filename)
            {
                save_shape_blitz(filename.value(), _shape_containers, to<uint8_t>(_export_bit_depth));
            }
        }
        ToolTip("Export Blitz Shapes");
        ImGui::SameLine();

        ImGui::SetNextItemWidth(200);
        ImGui::SliderInt("##slider", &_export_bit_depth, 1, 8, "Output bit-depth: %d");
        ToolTip("Clamp shapes to this bit-depth");

        ImGui::PopStyleColor();
    }

    void shape_editor_tool::save_shapes(std::filesystem::path const& shapes_file_path) const
    {
        auto all_shapes = std::vector<MPG::simple_image>{};
        for (auto const& shape_container : _shape_containers)
        {
            for (auto const& shape : shape_container.shapes)
            {
                auto export_shape = MPG::crop(shape_container.image, shape.x, shape.y, shape.width, shape.height);
                export_shape = MPG::crop_palette(export_shape, to<uint8_t>(_export_bit_depth), 0);
                all_shapes.push_back(export_shape);
            }
        }

        MPG::save_blitz_shapes(shapes_file_path, all_shapes);
    }
}
