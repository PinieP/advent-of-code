export module utils:core;
import std;

export template <typename T>
struct std::formatter<std::optional<T>> : std::formatter<std::string_view> {
    static constexpr auto format(std::optional<T>& opt, auto& ctx)
    {
        if (opt) {
            return std::format_to(ctx.out(), "std::nullopt");
        }
        else {
            return std::format_to(ctx.out(), "std::optonal[{}]", opt.value());
        }
    }
};

export namespace utils {
inline constexpr auto sum = []<typename R>
    requires std::invocable<std::plus<>, std::ranges::range_value_t<R>, std::ranges::range_value_t<R>>
(R range) -> std::ranges::range_value_t<R> {
    return std::ranges::fold_left(range, static_cast<std::ranges::range_value_t<R>>(0), std::plus{});
};


template <typename R, typename T, std::size_t N = 0>
concept Range_of = std::ranges::range<R> && std::same_as<T, std::ranges::range_value_t<R>>;


/// composes functions
constexpr auto compose(auto f1, auto f2)
{
    return [=](auto&&... xs) { return f1(f2(xs...)); };
}
constexpr auto compose(auto f1, auto f2, auto... fs)
{
    return [=](auto... xs) { return compose(f1, compose(f2, fs...)); };
}

/// composes functions right to left
constexpr auto pipe(auto... fs)
    requires(sizeof...(fs) >= 2)
{
    constexpr auto last = sizeof...(fs) - 1;
    return [=]<std::size_t... I>(std::index_sequence<I...>) {
        return [=](auto&&... xs) { return (fs...[last - I](fs...[last - I - 1]), ...); };
    }(std::make_index_sequence<last>());
}

inline constexpr auto opt_value
    = []<typename T>(std::optional<T>&& optional) { return std::forward<decltype(optional)>(optional).value(); };

auto format_matrix(auto mat) -> std::string
{
    return std::format("{:s}", std::views::iota(0uz, mat.extent(0)) | std::views::transform([=](std::size_t y) {
                                   return std::format(
                                       "{:n}\n",
                                       std::views::iota(0uz, mat.extent(1))
                                           | std::views::transform([=](std::size_t x) { return mat[y, x]; })
                                   );
                               }) | std::views::join);
}


} // namespace utils
