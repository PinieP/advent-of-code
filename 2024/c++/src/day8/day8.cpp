import std;
import utils;

using namespace utils::assert;
namespace views = std::views;
namespace ranges = std::ranges;
using utils::pretty::Color;
using utils::pretty::colored;

using Text_matrix = std::mdspan<const char, std::dextents<std::size_t, 2>, std::layout_stride>;
using utils::Range_of;

auto get_md(std::string_view text) -> Text_matrix
{
    std::size_t width = text.find_first_of('\n');
    std::size_t height = text.size() / (width + 1);
    return Text_matrix{
        text.data(),
        std::layout_stride::mapping{
            std::dextents<std::size_t, 2>{height, width}, std::array<std::size_t, 2>{width + 1, 1}
        }
    };
}

struct Coordinate {
    std::int32_t x;
    std::int32_t y;
    auto operator<=>(const Coordinate&) const = default;
};

template <>
struct std::formatter<Coordinate> : std::formatter<std::string_view> {
    static constexpr auto format(Coordinate c, auto& ctx) { return std::format_to(ctx.out(), "({}, {})", c.x, c.y); }
};

auto points_on_line(Coordinate a, Coordinate b, std::dextents<std::size_t, 2> ext)
{
    if (a.x > b.x) {
        std::swap(a, b);
    }
    auto delta_x = b.x - a.x;
    auto delta_y = b.y - a.y;
    auto gcd = std::gcd(delta_x, delta_y);
    auto x_step = delta_x / gcd;
    auto y_step = delta_y / gcd;
    std::int32_t start = std::min(-(a.x / x_step), (a.y / y_step) % static_cast<std::int32_t>(ext.extent(0)));

    std::int32_t end = std::min((ext.extent(0) - a.y) / y_step, (ext.extent(1) - a.x) / x_step);

    return std::pair{delta_x / x_step, views::iota(start, end) | views::transform([=](std::int32_t n) {
                                           return std::pair{n, Coordinate{a.x + n * x_step, a.y + n * y_step}};
                                       })};
}


auto puzzle1(std::string_view text) -> std::uint64_t
{
    std::map<char, std::vector<Coordinate>> map;
    auto matrix = get_md(text);
    for (std::int32_t i = 0; i < matrix.extent(0); ++i) {
        for (std::int32_t j = 0; j < matrix.extent(1); ++j) {
            if (char c = matrix[i, j]; map.contains(c)) {
                map.at(c).push_back(Coordinate{i, j});
            }
            else {
                map[c] = std::vector{Coordinate{i, j}};
            }
        }
    }
    map.extract('.');
    auto x = map | views::values //
           | views::transform(([&](std::span<Coordinate> r) {
                 return r | utils::enumerate | views::transform([=](auto tup) {
                            auto [i, coord_a] = tup;
                            return r | views::drop(i + 1) | views::transform([&](Coordinate coord_b) {
                                       auto [b_index, points] = points_on_line(coord_a, coord_b, matrix.extents());
                                       return points | views::transform([=](auto pair) -> std::optional<Coordinate> {
                                                  auto [index, coord] = pair;
                                                  auto dist_a = std::abs(index);
                                                  auto dist_b = std::abs(index - b_index);
                                                  return b_index * 2 == dist_b || dist_a == dist_b * 2
                                                           ? std::optional{coord}
                                                           : std::nullopt;
                                              });
                                   })
                                 | views::join;
                        })
                      | views::join;
             }))
           | views::join                                             //
           | views::filter([](auto opt) { return opt.has_value(); }) //
           | views::transform(utils::opt_value)                      //
           | ranges::to<std::vector>();
    std::ranges::sort(x);
    std::println("{}", x);
    return std::ranges::distance(x | views::chunk_by(std::equal_to{}));
    // return utils::sum(x);
}


auto puzzle2(std::string_view text) -> std::uint64_t { return -1; }


constexpr std::string_view test_input = R"(............
........0...
.....0......
.......0....
....0.......
......A.....
............
............
........A...
.........A..
............
............
)";

auto main() -> int
{
    const std::string input = [] {
        std::ostringstream stream;
        std::ifstream file("../inputs/8.txt");
        stream << file.rdbuf();
        return std::move(stream).str();
    }();

    assert_eq(puzzle1(test_input), 14);
    std::println("{}", colored(Color::green, "Test for puzzle 1 passed"));
    std::println("result of puzzle1 is: {}", puzzle1(input));

#if 0
    assert_eq(puzzle2(test_input), 11387);
    std::println("{}", colored(Color::green, "Test for puzzle 2 passed"));
    std::println("result of puzzle2 is: {}", puzzle2(input));
#endif
}
