#pragma once
#include <glfw/glfw3.h>
#include "glfw_utils.h"
#include <memory>

#include "shape_editor_tool.h"

struct ImFont;

namespace NEONnoir
{
    class editor
    {
    public:
        struct settings
        {
            std::string title;
            int32_t width{ 1600 };
            int32_t height{ 900 };
        };

    public:
        explicit editor(settings const& settings);
        ~editor() noexcept;

        void run();
        void load_fonts() noexcept;

        void process_main_menu();

    private:
        void save_project();

    private:
        GLFWwindow_ptr _window;
        ImFont* _ui_font;
        float _dpi_scale_x;
        float _dpi_scale_y;

        shape_editor_tool _shape_editor_tool{};

        bool _show_error_popup{ false };
        std::string _error_message{};
    };
};

