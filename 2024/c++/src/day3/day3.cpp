import std;
import utils;

using namespace assert;
using Parse_result = std::optional<std::string_view>;

auto parse_num(int& out)
{
    return [&out](std::string_view sv) -> Parse_result {
        const char* last = sv.end();
        auto res = std::from_chars(sv.data(), last, out);
        if (res.ec == std::errc::invalid_argument)
            return std::nullopt;
        else {
            return std::optional{std::string_view{res.ptr, last}};
        }
    };
}

auto parse_text(std::string_view pat)
{
    return [pat](std::string_view sv) -> Parse_result {
        // std::println("match: '{}' ? {}", pat, sv);
        return sv.starts_with(pat) ? std::optional{std::string_view{sv.begin() + pat.size(), sv.end()}} : std::nullopt;
    };
}

struct Parse_mul_value {
    std::string_view sv;
    int a;
    int b;
};
auto parse_mul(std::string_view sv) -> std::optional<Parse_mul_value>
{
    int a, b;
    return parse_text("mul(")(sv)
        .and_then(parse_num(a))
        .and_then(parse_text(","))
        .and_then(parse_num(b))
        .and_then(parse_text(")"))
        .transform([&](std::string_view sv) { return Parse_mul_value{sv, a, b}; });
}


constexpr auto drop_first = std::views::drop(1) | strings::to_sv;
auto puzzle1(std::string_view input) -> int
{
    return input.empty() ? 0
                         : parse_mul(input) //
                               .transform([](auto val) { return val.a * val.b + puzzle1(val.sv); })
                               .or_else([=] { return std::optional{puzzle1(input | drop_first)}; })
                               .value();
}


auto next_state(std::string_view sv, bool current) -> std::pair<std::string_view, bool>
{
    return parse_text("don't")(sv) //
        .transform([](std::string_view sv) { return std::pair{sv, false}; })
        .or_else([=] { return parse_text("do")(sv).transform([](std::string_view sv) { return std::pair{sv, true}; }); }
        )
        .value_or(std::pair{sv, current});
}

auto parse_recursive2(std::string_view input, bool state) -> int
{
    auto [sv, new_state] = next_state(input, state);
    return sv.empty() ? 0
                      : parse_mul(input)
                            .transform([=](auto val) {
                                return (new_state ? val.a * val.b : 0) + parse_recursive2(val.sv, new_state);
                            })
                            .or_else([=] { return std::optional{parse_recursive2(input | drop_first, new_state)}; })
                            .value();
}

auto puzzle2(std::string_view input) -> int { return parse_recursive2(input, true); }


constexpr std::string_view test_input = R"(xmul(2,4)&mul[3,7]!^don't()_mul(5,5)+mul(32,64](mul(11,8)undo()?mul(8,5)))";
auto main() -> int
{
    assert_eq(puzzle1(test_input), 161);
    assert_eq(puzzle2(test_input), 48);

    const std::string input = [] {
        std::ostringstream stream;
        std::ifstream file("../inputs/3.txt");
        stream << file.rdbuf();
        return std::move(stream).str();
    }();

    std::println("result of puzzle1 is: {}", puzzle1(input));
    std::println("result of puzzle2 is: {}", puzzle2(input));
}
