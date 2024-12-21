export module utils:strings;
import std;
import :core;

namespace utils::strings {

export template <typename R>
concept Sv_like = std::ranges::contiguous_range<R> && Range_of<R, char>;

struct Split_view_sentinel {};

template <typename F>
concept StringViewPatternFunc = std::same_as<std::size_t, std::invoke_result_t<F, std::string_view>>;

template <typename F>
concept PatternFunc = std::predicate<F, char> || StringViewPatternFunc<F>;

template <typename T>
concept Pattern = std::same_as<char, T>             //
               || std::same_as<std::string_view, T> //
               || std::same_as<const char*, T>      //
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
            if (consumed != 0 || self.sub_end_pos == self.end) {
                consumed = std::max(consumed, 1uz);
                break;
            }
            ++self.sub_end_pos;
        }
        self.next_sub_begin_pos = self.sub_end_pos + consumed;
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


export template <StringViewPatternFunc T>
struct Split_view : std::ranges::view_interface<Split_view<T>> {
    using iterator = Split_view_iter<T>;
    using sentinel = Split_view_sentinel;
    iterator iter;
    sentinel sen;


    Split_view(std::string_view text, StringViewPatternFunc auto pattern) : iter{text, pattern} {}

    auto begin(this const Split_view& self) -> iterator { return self.iter; }
    auto end(this const Split_view& self) -> sentinel { return self.sen; }
};

static_assert(Range_of<std::string_view, char>);

constexpr auto bake_pattern(char pattern) -> PatternFunc auto
{
    return [pattern](Range_of<char> auto to_match) -> std::size_t {
        return !std::ranges::empty(to_match) && *std::ranges::begin(to_match) == pattern ? 1 : 0;
    };
}
constexpr auto bake_pattern(std::string_view pattern) -> PatternFunc auto
{
    return [pattern](Range_of<char> auto to_match) -> std::size_t {
        return !std::ranges::empty(to_match) && std::ranges::starts_with(to_match, pattern) ? pattern.size() : 0;
    };
}
constexpr auto bake_pattern(const char* pattern) -> PatternFunc auto { return bake_pattern(std::string_view{pattern}); }
constexpr auto bake_pattern(std::predicate<char> auto pattern) -> PatternFunc auto
{
    return [pattern](Range_of<char> auto to_match) -> std::size_t {
        return !std::ranges::empty(to_match) && pattern(*std::ranges::begin(to_match)) ? 1 : 0;
    };
}
template <PatternFunc Pat>
constexpr auto bake_pattern(Pat pattern) -> Pat
{
    return pattern;
}


template <Pattern P>
struct Split_adaptor_closure : std::ranges::range_adaptor_closure<Split_adaptor_closure<P>> {
    P pattern;

    constexpr auto operator()(this const Split_adaptor_closure& self, const Range_of<char> auto& text)
        -> Split_view<decltype(bake_pattern(self.pattern))>
    {
        return {text, bake_pattern(self.pattern)};
    }
};
static_assert(std::ranges::range<decltype(std::declval<Split_adaptor_closure<char>>()(std::declval<std::string_view>())
              )>);

export inline constexpr struct Split_adaptor {
    static constexpr auto operator()(std::string_view pattern) -> Split_adaptor_closure<std::string_view>
    {
        return Split_adaptor_closure{.pattern = pattern};
    }
    static constexpr auto operator()(char pattern) -> Split_adaptor_closure<char>
    {
        return Split_adaptor_closure{.pattern = pattern};
    }
    template <PatternFunc PatFunc>
    static constexpr auto operator()(PatFunc pattern) -> Split_adaptor_closure<PatFunc>
    {
        return Split_adaptor_closure{.pattern = pattern};
    }
} split;

inline constexpr auto is_whitespace = [](char c) -> bool { return std::isspace(c); };

export inline constexpr struct Trim_adaptor : std::ranges::range_adaptor_closure<Trim_adaptor> {
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
        return std::string_view{start_pos, end_pos + 1};
    }
} trim;

/// Returns how often a Pattern matches in a range of chars
export auto count_matches(Range_of<char> auto range, Pattern auto pattern) -> std::size_t
{
    auto baked = bake_pattern(pattern);
    auto iter = std::ranges::begin(range);
    auto end = std::ranges::end(range);
    std::size_t count = 0;
    while (iter != end) {
        std::size_t consumed = baked(std::ranges::subrange{iter, end});
        if (consumed != 0) {
            std::advance(iter, consumed);
            ++count;
            continue;
        }
        ++iter;
    }
    return count;
}

export constexpr auto match_any_of(Pattern auto pat)
{
    return [pat = bake_pattern(pat)](const Range_of<char> auto& chars) -> std::size_t {
        auto baked = bake_pattern(pat);
        auto iter = chars.begin();
        auto end = chars.end();
        std::size_t total_consumed = 0;
        std::size_t consumed = 0;
        while ((consumed = baked(std::ranges::subrange{iter, end}))) {
            std::advance(iter, consumed);
            total_consumed += consumed;
        }
        return total_consumed;
    };
}

export constexpr auto match_or(Pattern auto... pats)
{
    return [=](const Range_of<char> auto& chars) -> std::size_t {
        std::size_t consumed = 0;
        ((consumed = bake_pattern(pats)(chars)) || ...);
        return consumed;
    };
}
inline constexpr auto match_whitespace = match_any_of(is_whitespace);


struct Split_whitespace : std::ranges::range_adaptor_closure<Split_whitespace> {
    static constexpr auto operator()(std::string_view sv) -> auto { return sv | trim | split(match_whitespace); }
};

export inline constexpr Split_whitespace split_whitespace;


export inline constexpr struct To_sv : std::ranges::range_adaptor_closure<To_sv> {
    static constexpr auto operator()(const Sv_like auto& range) -> std::string_view
    {
        return std::string_view{std::ranges::data(range), static_cast<std::size_t>(std::ranges::distance(range))};
    }
} to_sv;

export template <std::integral I>
constexpr auto parse_num = [](const Sv_like auto& text) -> std::optional<I> {
    std::string_view sv = text | to_sv;
    const char* first = std::ranges::begin(sv);
    const char* last = std::ranges::end(sv);
    I i;
    auto res = std::from_chars(first, last, i);
    if (res.ec == std::errc::invalid_argument) {
        return std::nullopt;
    }
    else {
        return std::optional{i};
    }
};

export template <std::integral I>
inline constexpr auto parse_num_value = compose(opt_value, parse_num<I>);


} // namespace utils::strings
