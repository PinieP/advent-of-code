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

namespace strings {


struct Split_view_sentinel {};

template <typename F>
concept StringPatternFunc = std::same_as<std::size_t, std::invoke_result_t<F, std::string_view>>;

template <typename F>
concept PatternFunc = std::predicate<F, char> || StringPatternFunc<F>;

template <typename T>
concept Pattern = std::same_as<char, T>             //
               || std::same_as<std::string_view, T> //
               || PatternFunc<T>;

export template <Pattern PatFunc>
class Split_view_iter {
    PatFunc pattern_func;
    const char* sub_begin_pos;
    const char* sub_end_pos;
    const char* end;
    const char* next_sub_begin_pos;

public:
    using value_type = std::string_view;
    using difference_type = std::ptrdiff_t;

    constexpr Split_view_iter() {}

    constexpr Split_view_iter(const Split_view_iter& other)
        : pattern_func{other.pattern_func}, sub_begin_pos{other.sub_begin_pos}, sub_end_pos{other.sub_end_pos},
          end{other.end}, next_sub_begin_pos{other.next_sub_begin_pos}
    {
    }
    auto operator=(const Split_view_iter& other) -> Split_view_iter&
    {
        pattern_func = other.pattern_func;
        sub_begin_pos = other.sub_begin_pos;
        sub_end_pos = other.sub_end_pos;
        end = other.end;
        next_sub_begin_pos = other.next_sub_begin_pos;
    }

    Split_view_iter(std::string_view text, PatFunc pattern_func)
        : pattern_func{pattern_func}, sub_begin_pos{nullptr}, sub_end_pos{nullptr}, end{text.data() + text.size()},
          next_sub_begin_pos{text.data()}
    {
        ++(*this);
    }


    constexpr auto operator++(this Split_view_iter& self) -> Split_view_iter&
    {
        if (self.next_sub_begin_pos > self.end) {
            self.sub_end_pos = self.end + 1;
            return self;
        }

        self.sub_begin_pos = self.next_sub_begin_pos;
        self.sub_end_pos = self.sub_begin_pos;
        std::size_t consumed = 0;
        while (true) {
            auto remainder = std::string_view{self.sub_end_pos, self.end};
            consumed = self.pattern_func(remainder);
            if (consumed != 0 || self.sub_end_pos == self.end) break;
            ++self.sub_end_pos;
        }
        self.next_sub_begin_pos = self.sub_end_pos + std::max(consumed, 1uz);
        return self;
    }


    constexpr auto operator*(this const Split_view_iter& self) -> value_type
    {
        return {self.sub_begin_pos, self.sub_end_pos};
    }

    constexpr auto operator++(this Split_view_iter& self, int) -> Split_view_iter
    {
        auto copy = self;
        ++self;
        return copy;
    }

    friend auto operator==(const Split_view_iter& iter, const Split_view_sentinel) -> bool
    {
        return iter.sub_end_pos > iter.end;
    }
    friend auto operator==(const Split_view_iter& lhs, const Split_view_iter& rhs) -> bool
    {
        lhs.sub_begin_pos == rhs.sub_begin_pos&& lhs.sub_end_pos == rhs.sub_end_pos&& lhs.end == rhs.end;
    }
};

export template <StringPatternFunc T>
struct Split_view : std::ranges::view_interface<Split_view<T>> {
    using iterator = Split_view_iter<T>;
    using sentinel = Split_view_sentinel;
    iterator iter;
    sentinel sen;


    Split_view(std::string_view text, StringPatternFunc auto pattern) : iter{text, pattern} {}

    auto begin(this const Split_view& self) -> iterator { return self.iter; }
    auto end(this const Split_view& self) -> sentinel { return self.sen; }
};

template <Pattern P>
struct Split_adaptor_closure : std::ranges::range_adaptor_closure<Split_adaptor_closure<P>> {
    P pattern;

    constexpr auto operator()(this const Split_adaptor_closure& self, std::string_view text)
        requires std::same_as<char, P>
    {
        const auto func = [pattern = self.pattern](std::string_view to_match) -> std::size_t {
            return !to_match.empty() && *to_match.begin() == pattern ? 1 : 0;
        };
        return Split_view<decltype(func)>{text, func};
    }

    constexpr auto operator()(this const Split_adaptor_closure& self, std::string_view text)
        requires std::same_as<std::string_view, P>
    {
        const auto func = [pattern = self.pattern](std::string_view to_match) -> std::size_t {
            return !to_match.empty() && to_match.starts_with(pattern) ? to_match.size() : 0;
        };
        return Split_view<decltype(func)>{text, func};
    }

    constexpr auto operator()(this const Split_adaptor_closure& self, std::string_view text)
        requires std::predicate<P, char>
    {
        const auto func = [pattern = self.pattern](std::string_view to_match) -> std::size_t {
            return !to_match.empty() && pattern(*to_match.begin()) ? 1 : 0;
        };
        return Split_view<decltype(func)>{text, func};
    }
    constexpr auto operator()(this const Split_adaptor_closure& self, std::string_view text)
        requires StringPatternFunc<P>
    {
        return Split_view<decltype(self.pattern)>{text, self.pattern};
    }
};

struct Split_adaptor {
    template <Pattern Pat>
    static constexpr auto operator()(std::string_view text, Pat pattern) -> auto
    {
        return Split_adaptor_closure{.pattern = pattern}(text);
    }
    template <Pattern Pat>
    static constexpr auto operator()(Pat pattern) -> auto
    {
        return Split_adaptor_closure{.pattern = pattern};
    }
};

export inline constexpr Split_adaptor split;

constexpr auto is_whitespace(char c) -> bool { return std::isspace(c); }

struct Trim_adaptor : std::ranges::range_adaptor_closure<Trim_adaptor> {
    static constexpr auto operator()(std::string_view sv) -> std::string_view
    {
        const char* start_pos = nullptr;
        for (const char& c : sv) {
            if (!is_whitespace(c)) {
                start_pos = &c;
                break;
            }
        }
        if (start_pos == nullptr) {
            return std::string_view{sv.data(), sv.data()};
        }
        const char* end_pos = nullptr;
        for (const char& c : sv | std::views::reverse) {
            if (!is_whitespace(c)) {
                end_pos = &c;
                break;
            }
        }
        return std::string_view{start_pos, end_pos};
    }
};
export inline constexpr Trim_adaptor trim;

constexpr auto match_whitespace(std::string_view sv) -> std::size_t
{
    return std::ranges::distance(sv | std::views::take_while(is_whitespace));
}

struct Split_whitespace : std::ranges::range_adaptor_closure<Split_whitespace> {
    static constexpr auto operator()(std::string_view sv) -> auto { return sv | trim | split(match_whitespace); }
};
export inline constexpr Split_whitespace split_whitespace;

template <typename R>
concept Sv_like = std::ranges::contiguous_range<R> && std::same_as<char, std::ranges::range_value_t<R>>;

struct To_sv : std::ranges::range_adaptor_closure<To_sv> {
    static constexpr auto operator()(const Sv_like auto& range) -> std::string_view
    {
        return std::string_view{std::ranges::data(range), static_cast<std::size_t>(std::ranges::distance(range))};
    }
};
export inline constexpr To_sv to_sv;

template <typename I>
    requires std::integral<I>
constexpr auto parse_num(const Sv_like auto& sv) -> std::optional<I>
{
    const char* first = std::ranges::data(sv);
    const char* last = first + std::ranges::distance(sv);
    I i;
    auto res = std::from_chars(first, last, i);
    if (res.ec == std::errc::invalid_argument) {
        return std::nullopt;
    }
    else {
        return std::optional{i};
    }
}


static_assert(std::ranges::range<decltype(std::declval<Split_adaptor_closure<char>>()(std::declval<std::string_view>())
              )>);

} // namespace strings
