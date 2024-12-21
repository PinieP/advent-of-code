import std;
import utils;

using namespace utils::assert;
namespace views = std::views;
namespace ranges = std::ranges;
namespace str = utils::strings;
using utils::pretty::Color;
using utils::pretty::colored;

using Text_matrix = std::mdspan<const char, std::dextents<std::size_t, 2>, std::layout_stride>;
using utils::Range_of;

using Slot = std::optional<std::size_t>;

auto parse(std::string_view text) -> std::vector<Slot>
{
    //
    auto sizes = text                                                    //
               | str::trim                                               //
               | views::transform([](char c) { return std::string{c}; }) //
               | views::transform(str::parse_num<std::size_t>)           //
               | views::transform(utils::opt_value);
    auto ids = views::iota(0uz)
             | views::transform([](std::size_t i) { return (i % 2 == 1) ? std::nullopt : std::optional{i / 2}; });
    return views::zip(sizes, ids)
         | views::transform([](auto pair) { return views::repeat(pair.second, pair.first); }) //
         | views::join                                                                        //
         | ranges::to<std::vector>();
}


auto compact(std::span<Slot> slots)
{
    auto iter = slots.begin();
    const auto end = slots.end();
    for (Slot& slot : slots | views::reverse) {
        if (!slot) continue;
        for (; iter < end; ++iter) {
            auto& free = *iter;
            if (&free >= &slot) return;
            if (free) continue;
            free.swap(slot);
            break;
        }
    }
}

auto checksum(utils::Range_of<Slot> auto&& slots) -> std::uint64_t
{
    return utils::sum(slots | utils::enumerate | views::transform([](auto pair) {
                          auto [pos, id] = pair;
                          return pos * id.value_or(0);
                      }));
}
auto puzzle1(std::string_view text) -> std::uint64_t
{
    auto slots = parse(text);
    compact(slots);
    return checksum(slots);
}

auto compact2(std::span<std::pair<std::size_t, Slot>> chunks)
{
    for (auto& slot : chunks | views::reverse) {
        if (!slot.second) continue;
        for (auto& swap : ranges::subrange{chunks.data(), &slot}) {
            if (!swap.second && swap.first <= slot.first) {
            }
        }
    }
}

using Chunk = std::pair<std::size_t, Slot>;

auto puzzle2(std::string_view text) -> std::uint64_t
{
    auto slots = parse(text);
    auto chunks = slots //
                | views::chunk_by(std::equal_to{})
                | views::transform([](auto chunk) -> Chunk { return {ranges::distance(chunk), *chunk.begin()}; })
                | ranges::to<std::vector>();

    compact2(chunks);
    std::println("{}", chunks | views::transform([](auto pair) {
                           return views::repeat(pair.second, pair.first)
                                | views::transform([](auto e) { return e ? std::format("{}", e.value()) : "."; });
                       }) | views::join);
    return checksum(
        chunks                                                                               //
        | views::transform([](auto pair) { return views::repeat(pair.second, pair.first); }) //
        | views::join                                                                        //
    );
}


constexpr std::string_view test_input = R"(2333133121414131402
)";

auto main() -> int
{
    const std::string input = [] {
        std::ostringstream stream;
        std::ifstream file("../inputs/9.txt");
        stream << file.rdbuf();
        return std::move(stream).str();
    }();

    assert_eq(puzzle1(test_input), 1928);
    std::println("{}", colored(Color::green, "Test for puzzle 1 passed"));
    std::println("result of puzzle1 is: {}", puzzle1(input));

    assert_eq(puzzle2(test_input), 2858);
    std::println("{}", colored(Color::green, "Test for puzzle 2 passed"));
    std::println("result of puzzle2 is: {}", puzzle2(input));
}
