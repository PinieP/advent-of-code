import std;
import utils;

using namespace utils::assert;

auto parse(std::string_view input) -> std::vector<std::vector<int>>
{
    return input.substr(0, input.size() - 1) | std::views::split('\n') | std::views::transform([](auto&& line) {
               std::stringstream stream{line | std::ranges::to<std::string>()};
               std::vector<int> res;
               int x;
               while (stream >> x) {
                   res.push_back(x);
               }
               return res;
           })
         | std::ranges::to<std::vector>();
}

constexpr auto is_safe = [](std::ranges::range auto& report) -> bool {
    return std::ranges::distance(report | std::views::chunk_by([](int a, int b) {
                                     int diff = std::abs(a - b);
                                     return diff >= 1 && diff <= 3;
                                 }))
            == 1
        && (std::ranges::is_sorted(report, std::less{}) || std::ranges::is_sorted(report, std::greater{}));
};

auto puzzle1(std::string_view input) -> int { return std::ranges::count_if(parse(input), is_safe); }

auto options(std::span<const int> report) -> std::vector<std::vector<int>>
{
    auto indices = std::views::iota(0uz, report.size());
    return indices | std::views::transform([&](std::size_t exclude) {
               return std::views::zip(report, indices)
                    | std::views::filter([=](auto pair) { return pair.second != exclude; }) //
                    | std::views::keys                                                      //
                    | std::ranges::to<std::vector>();
           })
         | std::ranges::to<std::vector>();
}
auto puzzle2(std::string_view input) -> int
{
    return std::ranges::count(
        parse(input) | std::views::transform([](std::span<int> x) { return std::ranges::any_of(options(x), is_safe); }),
        true
    );
}


constexpr std::string_view test_input = R"(7 6 4 2 1
1 2 7 8 9
9 7 6 2 1
1 3 2 4 5
8 6 4 4 1
1 3 6 7 9
)";

auto main() -> int
{
    assert_eq(puzzle1(test_input), 2);
    assert_eq(puzzle2(test_input), 4);

    const std::string input = [] {
        std::ostringstream stream;
        std::ifstream file("../inputs/2.txt");
        stream << file.rdbuf();
        return std::move(stream).str();
    }();

    std::println("result of puzzle1 is: {}", puzzle1(input));
    std::println("result of puzzle2 is: {}", puzzle2(input));
}
