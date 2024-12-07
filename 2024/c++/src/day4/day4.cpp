import std;
import utils;

using namespace utils::assert;
using namespace std::string_view_literals;
namespace str = utils::strings;
using utils::Range_of;

using Text_matrix = std::mdspan<const char, std::dextents<std::size_t, 2>>;
using Mat_index = std::pair<std::size_t, std::size_t>;


auto count_occurrences(Range_of<char> auto text) -> std::size_t
{
    return str::count_matches(text, "XMAS") + str::count_matches(text | std::views::reverse, "XMAS");
}

auto count_occurrences(std::ranges::range auto range) -> std::size_t
{
    return utils::sum(range | std::views::transform([](Range_of<char> auto r) { return count_occurrences(r); }));
}


enum struct Diag_dir { down_left, down_right };
auto diagonals(Diag_dir direction, Text_matrix mat) -> auto
{
    auto height = mat.extent(0);
    auto width = mat.extent(1);
    auto indices = std::views::iota(0uz, std::max(width, height) * 2) //
                 | std::views::transform([=](std::size_t n) {
                       auto range = std::views::iota(0uz, n);
                       return std::views::zip(range | std::views::reverse, range) //
                            | std::views::transform([=](Mat_index pair) {
                                  return direction == Diag_dir::down_right
                                           ? pair
                                           : Mat_index{height - 1 - pair.first, pair.second};
                              });
                   })
                 | std::views::transform(
                       std::views::filter([=](auto pair) { return pair.first < height && pair.second < width; })
                       | std::views::transform([mat](auto pair) { return mat[pair.first, pair.second]; })
                 );
    return indices;
}


auto verticals(Text_matrix mat) -> auto // Range_of<Range_of<char>>
{
    return std::views::iota(0uz, mat.extent(1)) | std::views::transform([=](std::size_t x) {
               return std::views::iota(0uz, mat.extent(0))
                    | std::views::transform([=](std::size_t y) { return mat[y, x]; });
           });
}


auto puzzle1(std::string_view text) -> int
{
    std::size_t width = text.find_first_of('\n') + 1;
    std::size_t height = text.size() / width;
    Text_matrix mat{text.data(), height, width};
    // print_mat(mat);

    return count_occurrences(text)           //
         + count_occurrences(verticals(mat)) //
         + count_occurrences(diagonals(Diag_dir::down_right, mat))
         + count_occurrences(diagonals(Diag_dir::down_left, mat));
}

struct Sliding_window {
    Text_matrix source;
    std::size_t y_offset;
    std::size_t x_offset;

    auto operator[](std::size_t y, std::size_t x) const -> char
    {
        assert_lt(y + y_offset, source.extent(0));
        assert_lt(x + x_offset, source.extent(1));
        return source[y + y_offset, x + x_offset];
    }
    auto is_xmas(this Sliding_window self) -> bool
    {
        auto ms1 = std::string{self[0, 0], self[2, 2]};
        auto ms2 = std::string{self[0, 2], self[2, 0]};
        return self[1, 1] == 'A' && (ms1 == "MS" || ms1 == "SM") && (ms2 == "MS" || ms2 == "SM");
    }
};

auto puzzle2(std::string_view text) -> int
{
    std::size_t width = text.find_first_of('\n') + 1;
    std::size_t height = text.size() / width;
    Text_matrix mat{text.data(), height, width};

    auto xmases = std::views::iota(0uz, height - 2) //
                | std::views::transform([=](std::size_t y) {
                      return std::views::iota(0uz, width - 2) //
                           | std::views::transform([=](std::size_t x) { return Sliding_window{mat, y, x}.is_xmas(); });
                  })
                | std::views::join;
    return std::ranges::count(xmases, true);
}


constexpr std::string_view test_input = R"(MMMSXXMASM
MSAMXMSMSA
AMXSXMAAMM
MSAMASMSMX
XMASAMXAMM
XXAMMXXAMA
SMSMSASXSS
SAXAMASAAA
MAMMMXMMMM
MXMXAXMASX
)";

auto main() -> int
{
    assert_eq(puzzle1(test_input), 18);
    assert_eq(puzzle2(test_input), 9);


    const std::string input = [] {
        std::ostringstream stream;
        std::ifstream file("../inputs/4.txt");
        stream << file.rdbuf();
        return std::move(stream).str();
    }();
    std::println("result of puzzle1 is: {}", puzzle1(input));
    std::println("result of puzzle2 is: {}", puzzle2(input));
}
