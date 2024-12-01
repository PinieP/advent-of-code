import std;
import utils;

using namespace assert;

auto read_file() -> std::string
{
    constexpr auto path = "src/day1/input.txt";
    std::ostringstream stream;
    std::ifstream file(path);
    stream << file.rdbuf();
    return stream.str();
}

template <typename T>
concept CharRange = std::ranges::range<T> && std::same_as<char, std::ranges::range_value_t<T>>;

auto parse_num(CharRange auto s) -> int
{
    const std::string str = s | std::ranges::to<std::string>();
    return std::stoi(str);
};


auto parse(std::string_view input) -> std::pair<std::vector<int>, std::vector<int>>
{
    constexpr auto eat_num = std::views::take_while([](char c) { return !std::isspace(c); });

    auto parsed
        = input                              //
        | std::views::take(input.size() - 1) //
        | std::views::split('\n')            //
        | std::views::transform([&](auto line) {
              return std::pair{
                  parse_num(line | eat_num), parse_num(line | std::views::reverse | eat_num | std::views::reverse)
              };
          });
    return std::pair{
        parsed | std::views::keys | std::ranges::to<std::vector>(),
        parsed | std::views::values | std::ranges::to<std::vector>()
    };
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

    const std::string input = read_file();
    std::println("result of puzzle1 is: {}", puzzle1(input));
    std::println("result of puzzle2 is: {}", puzzle2(input));
}
