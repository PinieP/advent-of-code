import std;
import utils;

using namespace utils::assert;
namespace views = std::views;
namespace ranges = std::ranges;
namespace str = utils::strings;
using utils::pretty::Color;
using utils::pretty::colored;

struct Operation {
    std::size_t result;
    std::vector<std::size_t> operands;
};
auto parse(std::string_view text) -> std::vector<Operation>
{
    return text             //
         | str::trim        //
         | str::split('\n') //
         | views::transform(
               str::split(str::match_or(": ", ' ')) //
               | views::transform(str::parse_num_value<std::size_t>)
         )
         | views::transform([](auto r) {
               return Operation{
                   .result{*r.begin()}, //
                   .operands{
                       views::drop(r, 1) //
                       | ranges::to<std::vector>()
                   }
               };
           })
         | ranges::to<std::vector>();
}

auto factorial(std::size_t n) -> std::size_t
{
    return n <= 1 ? 1 : ranges::fold_left(views::iota(1uz, n + 1), 1uz, std::multiplies{});
}

constexpr auto n_nominal_coefficient
    = [](std::same_as<std::size_t> auto... ks) { return factorial((0 + ... + ks)) / (1 * ... * factorial(ks)); };

template <std::size_t I>
    requires(I >= 1)
auto counts(std::size_t start, std::size_t end)
{
    if constexpr (I == 1) {
        return views::single(std::tuple{end - start});
    }
    else {
        return views::iota(start, end + 1) | views::transform([=](std::size_t i) {
                   return counts<I - 1>(0, end - i)
                        | views::transform([=](auto tup) { return std::tuple_cat(std::tuple{i}, tup); });
               })
             | views::join;
    }
}

template <char... Cs>
auto permutations(std::size_t size) -> std::vector<std::vector<char>>
{
    std::vector<char> vec(size);
    constexpr auto N = sizeof...(Cs);
    // std::println("{}", counts<N>(0, size));
    return counts<N>(0, size) //
         | views::transform([&vec](auto tup) {
               auto start = 0;
               [&, start]<std::size_t... Is>(std::index_sequence<Is...>) mutable {
                   (..., [&](std::size_t i, char c) {
                       std::ranges::fill_n(vec.begin() + start, i, c);
                       start += i;
                   }(std::get<Is>(tup), Cs));
               }(std::make_index_sequence<N>());

               return views::repeat(false, std::apply(n_nominal_coefficient, tup)) //
                    | views::transform([&vec](bool) mutable {
                          auto _ = ranges::next_permutation(vec);
                          return vec;
                      });
           })          //
         | views::join //
         | ranges::to<std::vector>();
}

template <char... Cs>
auto get_calibration_result(auto func, std::span<const Operation> operations)
{
    return utils::sum(
        operations | views::transform([&](const Operation& op) {
            auto results
                = permutations<Cs...>(op.operands.size() - 1) | views::transform([&](std::span<const char> ops) {
                      return ranges::fold_left(views::zip(ops, op.operands | views::drop(1)), op.operands[0], func);
                  });

            return ranges::any_of(
                       results | ranges::to<std::vector>(),
                       [result = op.result](std::size_t res) { return res == result; }
                   )
                     ? op.result
                     : 0;
        })
    );
}

auto puzzle1(std::string_view text) -> std::uint64_t
{
    return get_calibration_result<'+', '*'>(
        [](std::size_t acc, std::pair<char, std::size_t> pair) {
            auto [op, operand] = pair;
            return op == '+' ? acc + operand : acc * operand;
        },
        parse(text)
    );
}

auto cat(std::size_t a, std::size_t b)
{
    auto digits_b = b == 0 ? 1 : static_cast<std::size_t>(std::log10(b)) + 1;
    return a * static_cast<std::size_t>(std::pow(10, digits_b)) + b;
}

auto puzzle2(std::string_view text) -> std::uint64_t
{
    std::println("{}", permutations<'a', 'b', 'c'>(2));
    return get_calibration_result<'+', '*', '|'>(
        [](std::size_t acc, std::pair<char, std::size_t> pair) {
            auto [op, operand] = pair;
            switch (op) {
            case '+':
                return acc + operand;
            case '*':
                return acc * operand;
            case '|':
                return cat(acc, operand);
            default:
                std::unreachable();
            }
        },
        parse(text)
    );
}


constexpr std::string_view test_input = R"(190: 10 19
3267: 81 40 27
83: 17 5
156: 15 6
7290: 6 8 6 15
161011: 16 10 13
192: 17 8 14
21037: 9 7 18 13
292: 11 6 16 20
)";

auto main() -> int
{

    const std::string input = [] {
        std::ostringstream stream;
        std::ifstream file("../inputs/7.txt");
        stream << file.rdbuf();
        return std::move(stream).str();
    }();

#if 1
    assert_eq(puzzle1(test_input), 3749);
    std::println("{}", colored(Color::green, "Test for puzzle 1 passed"));
    std::println("result of puzzle1 is: {}", puzzle1(input));
#endif

    assert_eq(puzzle2(test_input), 11387);
    std::println("{}", colored(Color::green, "Test for puzzle 2 passed"));
    std::println("result of puzzle2 is: {}", puzzle2(input));
}
