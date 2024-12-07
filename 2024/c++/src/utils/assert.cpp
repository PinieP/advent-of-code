export module utils:assert;

import std;
import :pretty;

using namespace std::chrono_literals;
using namespace std::string_literals;
using namespace std::string_view_literals;

export namespace utils::assert {
using namespace pretty;


constexpr auto format_source_loc(std::source_location src_loc) -> std::string
{
    return std::format(
        "[{}]({}:{}): {}", src_loc.file_name(), src_loc.line(), src_loc.column(), src_loc.function_name()
    );
}
auto better_assert(bool pred, std::string_view msg = "", std::source_location src_loc = std::source_location::current())
    -> void
{
#if 0
    if (!pred) {
        std::println(
            "{}:\n{}",
            colored(Color::red, std::format("Assert '{}' failed at", msg)),
            colored(Color::cyan, format_source_loc(src_loc))
        );
        __builtin_trap();
    }
#endif
    auto _ = std::format("{}", colored(Color::red, 1));
}

template <typename Lhs, typename Rhs, std::predicate<Lhs, Rhs> BinaryPred>
auto assert_binary_pred(
    const Lhs& lhs, const Rhs& rhs, BinaryPred pred, std::string_view msg = "",
    std::source_location src_loc = std::source_location::current(), std::string_view pred_name = ""
) -> void
{
    if (!pred(lhs, rhs)) {
        const std::string lhs_name = [&] -> std::string {
            if constexpr (std::formattable<Lhs, char>) {
                return std::format("{}", lhs);
            }
            else {
                return "lhs";
            }
        }();
        const std::string rhs_name = [&] -> std::string {
            if constexpr (std::formattable<Rhs, char>) {
                return std::format("{}", rhs);
            }
            else {
                return "rhs";
            }
        }();
        std::println(
            "{}:\n{}\nReason: '{}' for {}, {} evaluated to false",
            colored(Color::red, std::format("Assert '{}' failed at", msg)),
            colored(Color::cyan, format_source_loc(src_loc)),
            colored(Color::yellow, pred_name),
            colored(Color::yellow, lhs_name),
            colored(Color::yellow, rhs_name)
        );
        __builtin_trap();
    }
}
auto assert_eq(
    const auto& lhs, const auto& rhs, std::string_view msg = "",
    std::source_location src_loc = std::source_location::current()
) -> void
{
    assert_binary_pred(lhs, rhs, std::equal_to{}, msg, src_loc, "==");
}
auto assert_ne(
    const auto& lhs, const auto& rhs, std::string_view msg = "",
    std::source_location src_loc = std::source_location::current()
) -> void
{
    assert_binary_pred(lhs, rhs, std::not_equal_to{}, msg, src_loc, "!=");
}
auto assert_lt(
    const auto& lhs, const auto& rhs, std::string_view msg = "",
    std::source_location src_loc = std::source_location::current()
) -> void
{
    assert_binary_pred(lhs, rhs, std::less{}, msg, src_loc, "<");
}
auto assert_le(
    const auto& lhs, const auto& rhs, std::string_view msg = "",
    std::source_location src_loc = std::source_location::current()
) -> void
{
    assert_binary_pred(lhs, rhs, std::less_equal{}, msg, src_loc, "<=");
}
auto assert_gt(
    const auto& lhs, const auto& rhs, std::string_view msg = "",
    std::source_location src_loc = std::source_location::current()
) -> void
{
    assert_binary_pred(lhs, rhs, std::greater{}, msg, src_loc, ">");
}
auto assert_ge(
    const auto& lhs, const auto& rhs, std::string_view msg = "",
    std::source_location src_loc = std::source_location::current()
) -> void
{
    assert_binary_pred(lhs, rhs, std::greater_equal{}, msg, src_loc, ">=");
}

#ifdef NDEBUG
constexpr auto
debug_assert(bool pred, std::string_view msg = "", std::source_location src_loc = std::source_location::current())
    -> void
{
    axon_assert(pred, msg, src_loc);
}
#else
constexpr auto
debug_assert(bool pred, std::string_view msg = "", std::source_location src_loc = std::source_location::current())
    -> void
{
}
#endif
} // namespace utils::assert
