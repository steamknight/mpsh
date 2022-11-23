#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_stdlib.h"
#include "IconsMaterialDesign.h"
#include "imgui_utils.h"

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <filesystem>
#include <format>
#include <iostream>

#include "utils.h"
#include "editor.h"

namespace fs = std::filesystem;

namespace NEONnoir
{
    editor::editor(editor::settings const& setting)
    {
        // Initialize GLFW
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);

        _window = GLFWwindow_ptr{ glfwCreateWindow(setting.width, setting.height, setting.title.c_str(), nullptr, nullptr) };
        ensure_valid(_window.get(), "Unable to create GLFW window");

        glfwMakeContextCurrent(_window.get());

        // Initialize GLAD/OpenGL
        gladLoadGL((GLADloadfunc)glfwGetProcAddress);
        glViewport(0, 0, setting.width, setting.height);

        glfwSetWindowUserPointer(_window.get(), this);

        // Intitialize IMGUI
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        // Enable Docking
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigWindowsMoveFromTitleBarOnly = true;

        ImGui::StyleColorsDark();
        ImGui_ImplGlfw_InitForOpenGL(_window.get(), true);
        ImGui_ImplOpenGL3_Init("#version 330");
 
        load_fonts();
    }

    editor::~editor() noexcept
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        glfwTerminate();
    }

    void editor::load_fonts() noexcept
    {
        glfwGetWindowContentScale(_window.get(), &_dpi_scale_x, &_dpi_scale_y);

        // Load some custom fonts
        ImGuiIO& io = ImGui::GetIO(); (void)io;

        io.Fonts->Clear();

        _ui_font = io.Fonts->AddFontFromFileTTF("data/Roboto-Medium.ttf", 18 * _dpi_scale_x);

        // Add icons to the ui font
        auto config = ImFontConfig{};
        config.MergeMode = true;
        config.PixelSnapH = true;
        config.GlyphOffset = { 0, 4 };
        config.GlyphMinAdvanceX = 18;
        ImWchar const icon_range[] = { ICON_MIN_MD, ICON_MAX_16_MD, 0 };
        io.Fonts->AddFontFromFileTTF("data/MaterialIcons-Regular.ttf", 18 * _dpi_scale_x, &config, icon_range);

        io.Fonts->Build();

        ImGui_ImplGlfw_NewFrame();
    }

    void editor::run()
    {
        while (!glfwWindowShouldClose(_window.get()))
        {
            glClear(GL_COLOR_BUFFER_BIT);

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

            //process_main_menu();

            _shape_editor_tool.display();

            if (_show_error_popup)
            {
                ImGui::OpenPopup("Error");
                ImVec2 center = ImGui::GetMainViewport()->GetCenter();
                ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
                if (ImGui::BeginPopupModal("Error"))
                {
                    ImGui::Text(_error_message.c_str());

                    ImGui::NewLine();
                    auto width = ImGui::GetWindowWidth();
                    auto size = ImVec2{ 120.f * _dpi_scale_x, 0.f };

                    ImGui::Dummy({ width - size.x - 25.f, 0.f });
                    ImGui::SameLine();
                    if (ImGui::Button("Bummer", size))
                    {
                        _show_error_popup = false;
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::EndPopup();
                }
            }

            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            glfwSwapBuffers(_window.get());
            glfwPollEvents();
        }
    }

    void editor::save_project()
    {
    }

    void editor::process_main_menu()
    {
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("New", "Ctrl+N"))
                {
                }

                if (ImGui::MenuItem("Open", "Ctrl+O")) 
                {
                    auto file = open_file_dialog("json");
                    if (file)
                    {
                        try
                        {
                        }
                        catch (std::exception const& ex)
                        {
                            std::cout << ex.what();
                            _show_error_popup = true;
                            _error_message = ex.what();
                        }
                    }
                }

                if (ImGui::MenuItem("Save", "Ctrl+S")) 
                {
                    save_project();
                }
                if (ImGui::MenuItem("Save As..."))
                {
                    auto file = save_file_dialog("json");
                    if (file.has_value())
                    {
                    }
                }

                ImGui::Separator();
                if (ImGui::MenuItem("Export MPSH file..."))
                {
                    try
                    {
                    }
                    catch (std::exception const& ex)
                    {
                        std::cout << ex.what();
                        _show_error_popup = true;
                        _error_message = ex.what();
                    }
                }

                ImGui::Separator();
                if (ImGui::MenuItem("Exit", "Alt+F4"))
                {
                    glfwSetWindowShouldClose(_window.get(), GLFW_TRUE);
                }

                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }
    }
}
