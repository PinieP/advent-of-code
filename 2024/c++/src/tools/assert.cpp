export module assert;
import std;

constexpr auto better_assert(bool pred, std::string_view message) -> void {
  if (!pred) {
    std::println(std::cerr, "{}", message);
  }
}
