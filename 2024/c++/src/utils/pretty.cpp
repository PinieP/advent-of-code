export module utils:pretty;

import std;


namespace utils::pretty {
export template <std::formattable<char> T>
struct Pretty_print_info;
}

export template <typename T>
struct std::formatter<utils::pretty::Pretty_print_info<T>> : std::formatter<std::string_view> {
    static constexpr auto format(const utils::pretty::Pretty_print_info<T>& text, auto& ctx)
    {
        return std::format_to(
            ctx.out(),
            "\033[{};{}m{}\033[0m",
            std::to_underlying(text.text_type),
            std::to_underlying(text.color),
            text.text
        );
    }
};


export namespace utils::pretty {

enum struct Color { black = 30, red = 31, green = 32, yellow = 33, blue = 34, magenta = 35, cyan = 36, white = 37 };
enum struct Text_type { normal = 0, bold = 1, underlined = 4 };

template <std::formattable<char> T>
struct Pretty_print_info {
    T text;
    Color color = Color::white;
    Text_type text_type = Text_type::normal;
};

template <std::formattable<char> T>
[[nodiscard]] constexpr auto colored(Color color, T text) -> Pretty_print_info<T>
{
    return Pretty_print_info{text, color, Text_type::normal};
}

template <typename T>
[[nodiscard]] constexpr auto colored(Color color, Pretty_print_info<T> text) -> Pretty_print_info<T>
{
    text.color = color;
    return text;
}

[[nodiscard]] constexpr auto operator""_colored(const char* ptr, std::size_t size)
{
    return [=](Color color) { return colored(color, std::string_view{ptr, size}); };
};

template <std::formattable<char> T>
[[nodiscard]] constexpr auto bold(T text)
{
    return Pretty_print_info{text, Color::white, Text_type::bold};
}

template <typename T>
[[nodiscard]] constexpr auto bold(Pretty_print_info<T> text) -> Pretty_print_info<T>
{
    text.text_type = Text_type::bold;
    return text;
}

[[nodiscard]] constexpr auto operator""_bold(const char* ptr, std::size_t size)
{
    return bold(std::string_view{ptr, size});
}

} // namespace utils::pretty
