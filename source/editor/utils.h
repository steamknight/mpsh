#pragma once
#include <optional>
#include <string_view>

namespace NEONnoir
{
    std::optional<std::string_view> open_file_dialog(std::string_view const& filter);
    std::optional<std::string_view> save_file_dialog(std::string_view const& filter);

    template<char... T>
    size_t operator"" _z()
    {
        return static_cast<size_t>(T);
    }

    template<typename T, typename U>
    auto to(U value) -> T
    {
        return static_cast<T>(value);
    }


    template<typename T, typename U>
    auto force_to(U value) -> T
    {
        return reinterpret_cast<T>(value);
    }

    void write(std::ofstream& stream, uint16_t value);
    void write(std::ofstream& stream, uint32_t value);
}