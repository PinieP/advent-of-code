import std;
import utils;

using namespace utils::assert;
namespace str = utils::strings;
namespace views = std::views;
namespace ranges = std::ranges;
using namespace std::string_view_literals;
using utils::pretty::Color;
using utils::pretty::colored;

constexpr auto range_to_pair = [](ranges::range auto range) {
    auto iter = ranges::begin(range);
    auto first = iter;
    auto second = ++iter;
    assert_eq(++iter, ranges::end(range));
    auto pair = std::pair{*first, *second};
    return pair;
};
auto parse(std::string_view text) -> std::pair<std::set<std::pair<int, int>>, std::vector<std::vector<int>>>
{
    constexpr auto parse_ints = views::transform(str::parse_num<int>) | views::transform(utils::opt_value);
    const auto [constraint_text, updates_text] = range_to_pair(text | str::split("\n\n"));
    const auto constraints = constraint_text //
                           | str::split('\n')
                           | views::transform(
                                 str::split('|') //
                                 | parse_ints
                           )
                           | views::transform(range_to_pair) //
                           | ranges::to<std::set>();
    const auto updates = updates_text //
                       | str::split_whitespace
                       | views::transform(
                             str::split(',') //
                             | parse_ints    //
                             | ranges::to<std::vector>()
                       )
                       | ranges::to<std::vector>();
    return std::pair{constraints, updates};
}

auto get_sort_func(const std::set<std::pair<int, int>>& constraints)
{
    return [&constraints](int a, int b) { return constraints.contains(std::pair{a, b}); };
}
auto middle_point(std::span<const int> span) -> int { return span[(span.size() - 1) / 2]; }

auto puzzle1(std::string_view text) -> int
{
    const auto [constraints, updates] = parse(text);
    return utils::sum(
        updates //
        | views::filter([&](auto r) { return std::ranges::is_sorted(r, get_sort_func(constraints)); })
        | views::transform(middle_point)
    );
}

auto puzzle2(std::string_view text) -> int
{
    const auto [constraints, updates] = parse(text);
    auto sort_func = get_sort_func(constraints);
    return utils::sum(
        updates //
        | views::filter([&](auto r) { return !std::ranges::is_sorted(r, sort_func); })
        | views::transform([&](std::vector<int> vec) {
              std::ranges::sort(vec, sort_func);
              return vec;
          })
        | views::transform(middle_point)
    );
}


constexpr std::string_view test_input = R"(47|53
97|13
97|61
97|47
75|29
61|13
75|53
29|13
97|29
53|29
61|53
97|53
61|29
47|13
75|47
97|75
47|61
75|61
47|29
75|13
53|13

75,47,61,53,29
97,61,53,29,13
75,29,13
75,97,47,61,53
61,13,29
97,13,75,29,47
)";

auto main() -> int
{
    assert_eq(puzzle1(test_input), 143);
    std::println("{}", colored(Color::green, "Test for puzzle 1 passed"));
    assert_eq(puzzle2(test_input), 123);
    std::println("{}", colored(Color::green, "Test for puzzle 2 passed"));

    const std::string input = [] {
        std::ostringstream stream;
        std::ifstream file("../inputs/5.txt");
        stream << file.rdbuf();
        return std::move(stream).str();
    }();
    std::println("result of puzzle1 is: {}", puzzle1(input));
    std::println("result of puzzle2 is: {}", puzzle2(input));
}
