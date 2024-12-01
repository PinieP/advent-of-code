export module utils;

import std;

namespace pretty {

export template <std::formattable<char> T>
struct Pretty_print_info;
}

export template <typename T>
struct std::formatter<pretty::Pretty_print_info<T>> : std::formatter<std::string_view> {
    constexpr auto format(const pretty::Pretty_print_info<T>& text, auto& ctx) const
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

export namespace pretty {

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

template <typename T>
concept Error = requires(T t) {
    { t.description() } -> std::formattable<char>;
    { t.context_description() } -> std::formattable<char>;
};
struct Default_error {
    [[nodiscard]] constexpr auto description() const -> std::string_view { return "Something bad happened"; }
    [[nodiscard]] constexpr auto context_description() const -> std::string_view { return "in axon"; }
    constexpr auto action() const -> void {}
};
auto report_error(Error auto error, bool terminate = true) -> void
{
    std::println(
        "{}\nA fatal error occured while {}\n{}",
        colored(Color::red, "Error:"_bold),
        error.context_description(),
        error.description()
    );
    if (terminate) {
        std::exit(1);
    }
}

} // namespace pretty

constexpr auto format_source_loc(std::source_location src_loc) -> std::string
{
    return std::format(
        "[{}]({}:{}): {}", src_loc.file_name(), src_loc.line(), src_loc.column(), src_loc.function_name()
    );
}

export namespace assert {
using namespace std::chrono_literals;
using namespace std::string_literals;
using namespace std::string_view_literals;
using namespace ::pretty;


auto better_assert(bool pred, std::string_view msg = "", std::source_location src_loc = std::source_location::current())
    -> void
{
    if (!pred) {
        std::println(
            "{}:\n{}",
            colored(Color::red, std::format("Assert '{}' failed at", msg)),
            colored(Color::cyan, format_source_loc(src_loc))
        );
        __builtin_trap();
    }
}

template <typename Lhs, typename Rhs, std::predicate<Lhs, Rhs> BinaryPred>
auto assert_binary_pred(
    const Lhs& lhs, const Rhs& rhs, BinaryPred pred, std::string_view msg = "",
    std::source_location src_loc = std::source_location::current(), std::string_view pred_name = ""
) -> void
{
    if (!pred(lhs, rhs)) {
        const std::string lhs_name = [&] -> std::string {
            if constexpr (std::formattable<Lhs, char>) {
                return std::format("{}", lhs);
            }
            else {
                return "lhs";
            }
        }();
        const std::string rhs_name = [&] -> std::string {
            if constexpr (std::formattable<Rhs, char>) {
                return std::format("{}", rhs);
            }
            else {
                return "rhs";
            }
        }();
        std::println(
            "{}:\n{}\nReason: '{}' for {}, {} evaluated to false",
            colored(Color::red, std::format("Assert '{}' failed at", msg)),
            colored(Color::cyan, format_source_loc(src_loc)),
            colored(Color::yellow, pred_name),
            colored(Color::yellow, lhs_name),
            colored(Color::yellow, rhs_name)
        );
        __builtin_trap();
    }
}
auto assert_eq(
    const auto& lhs, const auto& rhs, std::string_view msg = "",
    std::source_location src_loc = std::source_location::current()
) -> void
{
    assert_binary_pred(lhs, rhs, std::equal_to{}, msg, src_loc, "==");
}
auto assert_ne(
    const auto& lhs, const auto& rhs, std::string_view msg = "",
    std::source_location src_loc = std::source_location::current()
) -> void
{
    assert_binary_pred(lhs, rhs, std::not_equal_to{}, msg, src_loc, "!=");
}
auto assert_lt(
    const auto& lhs, const auto& rhs, std::string_view msg = "",
    std::source_location src_loc = std::source_location::current()
) -> void
{
    assert_binary_pred(lhs, rhs, std::less{}, msg, src_loc, "<");
}
auto assert_le(
    const auto& lhs, const auto& rhs, std::string_view msg = "",
    std::source_location src_loc = std::source_location::current()
) -> void
{
    assert_binary_pred(lhs, rhs, std::less_equal{}, msg, src_loc, "<=");
}
auto assert_gt(
    const auto& lhs, const auto& rhs, std::string_view msg = "",
    std::source_location src_loc = std::source_location::current()
) -> void
{
    assert_binary_pred(lhs, rhs, std::greater{}, msg, src_loc, ">");
}
auto assert_ge(
    const auto& lhs, const auto& rhs, std::string_view msg = "",
    std::source_location src_loc = std::source_location::current()
) -> void
{
    assert_binary_pred(lhs, rhs, std::greater_equal{}, msg, src_loc, ">=");
}

#ifdef NDEBUG
constexpr auto
debug_assert(bool pred, std::string_view msg = "", std::source_location src_loc = std::source_location::current())
    -> void
{
    axon_assert(pred, msg, src_loc);
}
#else
constexpr auto
debug_assert(bool pred, std::string_view msg = "", std::source_location src_loc = std::source_location::current())
    -> void
{
}
#endif
} // namespace assert
