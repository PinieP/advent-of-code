#include "util.hpp"
#include <cstddef>
#include <print>

using example = string_to_list<"987654321111111\n"
                               "811111111111119\n"
                               "234234234234278\n"
                               "818181911112111">;

template <typename xs>
using parse = split_on<V<'\n'>, xs>;

template <std::size_t n, typename xs>
struct Count {

    using part = take<xs::len - n + 1, xs>;
    static_assert(!std::same_as<part, List<>>);
    using max_elem = maximum<take<xs::len - n + 1, xs>>;
    using Value = prepend<max_elem, typename Count<n - 1, tail<drop_until_eq<max_elem, xs>>>::Value>;
};
template <typename xs>
struct Count<0, xs> {
    using Value = List<>;
};

template <std::size_t n, typename xs>
struct Counts {
    template <typename lst>
    using countn = Count<n, lst>::Value;
    using Value = map<parse_int, map<countn, xs>>;
};
template <std::size_t n, typename xs>
using counts = Counts<n, xs>::Value;

template <typename xs>
using part1 = sum<counts<2, xs>>;
template <typename xs>
using part2 = sum<counts<12, xs>>;

#if 0
constexpr char inputs[] =
{
#embed "../../inputs/3.txt"
};
using inp = string_to_list<inputs>;
#endif


int main()
{
    using t1 = part1<parse<example>>;
    using t2 = part2<parse<example>>;
    std::println("Part1:\n{}\nPart2:{}\n", t1{}, t2{});
}
