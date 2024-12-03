import std;
import utils;

using namespace assert;


auto parse(std::string_view input) -> std::pair<std::vector<int>, std::vector<int>>
{
    std::stringstream stream{input};
    std::vector<int> left, right;
    int a, b;
    while (stream >> a >> b) {
        left.push_back(a);
        right.push_back(b);
    }
    return {left, right};
}


auto puzzle1(std::string_view input) -> int
{
    auto [left, right] = parse(input);

    std::ranges::sort(left);
    std::ranges::sort(right);

    return std::ranges::fold_left(
        std::views::zip(left, right)
            | std::views::transform([](std::pair<int, int> pair) { return std::abs(pair.first - pair.second); }),
        0,
        std::plus{}
    );
}

auto puzzle2(std::string_view input) -> int
{
    auto [left, right] = parse(input);
    std::ranges::sort(right);
    const auto map = right //
                   | std::views::chunk_by(std::equal_to{})
                   | std::views::transform([](auto x) { return std::pair{*x.begin(), std::ranges::distance(x)}; })
                   | std::ranges::to<std::unordered_map>();
    return std::ranges::fold_left(
        left //
            | std::views::transform([&](const int e) {
                  auto it = map.find(e);
                  return e * (it != map.end() ? it->second : 0);
              }),
        0,
        std::plus{}
    );
}


constexpr std::string_view test_input = R"(3   4
4   3
2   5
1   3
3   9
3   3
)";

auto main() -> int
{
    assert_eq(puzzle1(test_input), 11);
    assert_eq(puzzle2(test_input), 31);

    const std::string input = [] {
        std::ostringstream stream;
        std::ifstream file("../inputs/1.txt");
        stream << file.rdbuf();
        return std::move(stream).str();
    }();
    std::println("result of puzzle1 is: {}", puzzle1(input));
    std::println("result of puzzle2 is: {}", puzzle2(input));
}
