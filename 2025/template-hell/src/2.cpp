#include "util.hpp"
#include <print>
#include <type_traits>


template <typename lst>
struct F {
    using split = split_first<V<'-'>, lst>;
    static constexpr auto from = parse_int<fst<split>>::value;
    static constexpr auto to = parse_int<snd<split>>::value;
    using ids = range<from, to>;

    template <typename id>
    struct M {
        using chars = int_to_string<id::value>;
        using split = split_at<chars::len / 2, chars>;
        static constexpr auto eq = std::same_as<fst<split>, snd<split>>;
        using Value = std::conditional_t<eq, id, V<0>>;
    };

    template <typename id>
    using m = M<id>::Value;

    static constexpr auto value = sum<map<m, ids>>::value;
};
template <typename lst>
using f = V<F<lst>::value>;


using example = string_to_list<"11-22,95-115,998-1012,1188511880-1188511890,222220-222224,1698522-1698528,446443-"
                               "446449,38593856-38593862,565653-565659,824824821-824824827,2121212118-2121212124">;

using split = split_on<V<','>, example>;


constexpr auto task1 = sum<map<f, split>>::value;


int main() { std::println("list: {}", task1); }
