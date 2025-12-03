#include <cassert>
#include <concepts>
#include <cstddef>
#include <format>
#include <string_view>
#include <type_traits>
#include <utility>

struct AbstractV {};


template <auto v>
struct V : AbstractV {
    static constexpr auto value = v;
};

template <typename... Elems>
struct List {
    static constexpr auto len = sizeof...(Elems);
};

template <auto... vals>
using list = List<V<vals>...>;

template <std::derived_from<AbstractV> T>
struct std::formatter<T> : std::formatter<std::string_view> {
    auto format(const T l, std::format_context& ctx) const { return std::format_to(ctx.out(), "{}", T::value); }
};


template <typename>
struct Fst;

template <typename Head, typename... Rest>
struct Fst<List<Head, Rest...>> {
    using Value = Head;
};

template <typename List>
using fst = Fst<List>::Value;

template <typename>
struct Snd;

template <typename F, typename S, typename... Rest>
struct Snd<List<F, S, Rest...>> {
    using Value = S;
};

template <typename List>
using snd = Snd<List>::Value;

template <typename>
struct Tail;

template <typename x, typename... xs>
struct Tail<List<x, xs...>> {
    using Value = List<xs...>;
};

template <>
struct Tail<List<>> {
    using Value = List<>;
};
template <typename xs>
using tail = Tail<xs>::Value;


template <typename first, typename rest>
struct Prepend;

template <typename first, typename... rest>
struct Prepend<first, List<rest...>> {
    using Value = List<first, rest...>;
};
template <typename first, typename rest>
using prepend = typename Prepend<first, rest>::Value;

template <typename, typename>
struct Append;

template <typename... start, typename last>
struct Append<List<start...>, last> {
    using Value = List<start..., last>;
};
template <typename first, typename rest>
using append = typename Append<first, rest>::Value;


template <typename... Rest>
struct std::formatter<List<Rest...>> : std::formatter<std::string_view> {
    auto format(const List<Rest...>& l, std::format_context& ctx) const
    {
        int idx = 0;
        constexpr auto size = sizeof...(Rest);
        std::format_to(ctx.out(), "[");
        (..., (std::format_to(ctx.out(), "{}", Rest{}), ++idx, *ctx.out()++ = (idx < size) ? ", " : ""));
        return std::format_to(ctx.out(), "]");
    }
};


template <template <typename> typename, typename>
struct TakeWhile;


template <typename T>
concept IsFalse = std::derived_from<T, std::false_type>;

template <typename T>
concept IsTrue = std::derived_from<T, std::true_type>;

template <template <typename> typename P, typename List>
using take_while = typename TakeWhile<P, List>::Value;

template <template <typename> typename P, typename first, typename... rest>
    requires(IsFalse<P<first>>)
struct TakeWhile<P, List<first, rest...>> {
    using Value = List<>;
};

template <template <typename> typename P, typename first, typename... rest>
    requires(IsTrue<P<first>>)
struct TakeWhile<P, List<first, rest...>> {
    using Value = prepend<first, take_while<P, List<rest...>>>;
};

template <template <typename> typename P>
struct TakeWhile<P, List<>> {
    using Value = List<>;
};


template <template <typename> typename P, typename>
struct SkipWhile {
    static_assert(false);
};

template <template <typename> typename P, typename List>
using skip_while = typename SkipWhile<P, List>::Value;


template <template <typename> typename P, typename x, typename... xs>
    requires IsTrue<P<x>>
struct SkipWhile<P, List<x, xs...>> {
    using Value = skip_while<P, List<xs...>>;
};

template <template <typename> typename P, typename x, typename... xs>
    requires IsFalse<P<x>>
struct SkipWhile<P, List<x, xs...>> {
    using Value = List<xs...>;
};

template <template <typename> typename P>
struct SkipWhile<P, List<>> {
    using Value = List<>;
};

template <typename>
struct Not;
template <IsTrue T>
struct Not<T> : std::false_type {};
template <IsFalse T>
struct Not<T> : std::true_type {};


template <typename sentinel, typename xs>
struct TakeUntilEq {
    template <typename U>
    using E = Not<std::is_same<sentinel, U>>;

    using Value = take_while<E, xs>;
};
template <typename sentinel, typename xs>
using take_until_eq = TakeUntilEq<sentinel, xs>::Value;

template <typename sentinel, typename xs>
struct SkipUntilEq {
    template <typename U>
    using E = Not<std::is_same<sentinel, U>>;

    using Value = skip_while<E, xs>;
};
template <typename sentinel, typename xs>
using skip_until_eq = SkipUntilEq<sentinel, xs>::Value;

template <typename delim, typename xs>
using split_first = List<take_until_eq<delim, xs>, skip_until_eq<delim, xs>>;

template <typename, typename>
struct SplitOn;

template <typename delim, typename xs>
using split_on = SplitOn<delim, xs>::Value;

template <typename delim, typename... xs>
struct SplitOn<delim, List<xs...>> {
    using pair = split_first<delim, List<xs...>>;
    using Value = prepend<fst<pair>, split_on<delim, snd<pair>>>;
};


template <typename delim>
struct SplitOn<delim, List<>> {
    using Value = List<>;
};


template <int acc, char... xs>
struct ParseIntAcc;

template <int acc, char x, char... xs>
struct ParseIntAcc<acc, x, xs...> {
    static constexpr auto value = ParseIntAcc<acc * 10 + x - '0', xs...>::value;
};
template <int acc>
struct ParseIntAcc<acc> {
    static constexpr auto value = acc;
};

template <char... xs>
struct ParseIntStart {
    static constexpr auto value = ParseIntAcc<0, xs...>::value;
};

template <char... xs>
struct ParseIntStart<'-', xs...> {
    static constexpr auto value = -ParseIntAcc<0, xs...>::value;
};

template <char... xs>
struct ParseIntStart<'+', xs...> {
    static constexpr auto value = ParseIntAcc<0, xs...>::value;
};


template <typename>
struct ParseInt;

template <char... xs>
struct ParseInt<List<V<xs>...>> {
    static constexpr auto value = ParseIntStart<xs...>::value;
};


template <typename lst>
constexpr std::int64_t parse_int = ParseInt<lst>::value;


template <std::size_t N>
struct fixed_string {
    char value[N];

    // deduction from string literal
    constexpr fixed_string(char const (&str)[N])
    {
        for (std::size_t i = 0; i < N; ++i)
            value[i] = str[i];
    }
};

template <fixed_string s>
using string_to_list = decltype([]<std::size_t... i>(std::index_sequence<i...>) {
    return List<V<s.value[i]>...>{};
}(std::make_index_sequence<sizeof(s.value) - 1>()));

template <template <typename> typename f, typename>
struct Map;

template <template <typename> typename f, typename xs>
using map = Map<f, xs>::Value;

template <template <typename> typename f, typename x, typename... xs>
struct Map<f, List<x, xs...>> {
    using Value = prepend<f<x>, map<f, List<xs...>>>;
};

template <template <typename> typename f>
struct Map<f, List<>> {
    using Value = List<>;
};


template <std::size_t n, typename>
struct TakeN;

template <std::size_t n, typename xs>
using take_n = TakeN<n, xs>::Value;

template <typename... xs>
struct TakeN<0, List<xs...>> {
    using Value = List<>;
};

template <std::size_t n, typename x, typename... xs>
    requires(n != 0)
struct TakeN<n, List<x, xs...>> {
    using Value = prepend<x, take_n<n - 1, List<xs...>>>;
};
template <std::size_t n>
struct TakeN<n, List<>> {
    using Value = List<>;
};

template <std::size_t n, typename>
struct DropN;

template <std::size_t n, typename xs>
using drop_n = DropN<n, xs>::Value;

template <std::size_t n, typename x, typename... xs>
    requires(n != 0)
struct DropN<n, List<x, xs...>> {
    using Value = drop_n<n - 1, List<xs...>>;
};
template <typename xs>
struct DropN<0, xs> {
    using Value = xs;
};
template <std::size_t n>
struct DropN<n, List<>> {
    using Value = List<>;
};

template <std::size_t n, typename xs>
using split_at = List<take_n<n, xs>, drop_n<n, xs>>;


template <std::size_t from, std::size_t to>
struct Range {
    using Value = prepend<V<from>, typename Range<from + 1, to>::Value>;
};
template <std::size_t to>
struct Range<to, to> {
    using Value = List<>;
};
template <std::size_t from, std::size_t to>
using range = Range<from, to>::Value;


template <std::int64_t n>
struct IntToString {
    static constexpr int div = n / 10;
    static constexpr int mod = n % 10;
    static constexpr char c = '0' + mod;
    using Value = append<typename IntToString<div>::Value, V<c>>;
};

template <std::int64_t n>
using int_to_string = IntToString<n>::Value;

template <std::int64_t n>
    requires(n < 0)
struct IntToString<n> {
    using Value = prepend<V<'-'>, int_to_string<-n>>;
};


template <>
struct IntToString<0> {
    using Value = List<>;
};

template <typename lst>
struct Sum;

template <auto x, auto... xs>
struct Sum<List<V<x>, V<xs>...>> {
    static constexpr auto value = x + Sum<List<V<xs>...>>::value;
};

template <>
struct Sum<List<>> {
    static constexpr auto value = 0;
};
template <typename lst>
static constexpr auto sum = Sum<lst>::value;
