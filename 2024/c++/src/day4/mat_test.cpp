import std;
auto print_mat(auto mat)
{
    std::println("Matrix:\n{:s}", std::views::iota(0uz, mat.extent(0)) | std::views::transform([=](std::size_t y) {
                                      return std::format(
                                          "{:n}\n",
                                          std::views::iota(0uz, mat.extent(1))
                                              | std::views::transform([=](std::size_t x) { return mat[y, x]; })
                                      );
                                  }) | std::views::join);
}

auto main() -> int
{
    auto x = std::array{1, 2, 3, 4, 5, 6, 7, 8};
    auto ma1 = std::mdspan{x.data(), 2, 4};
    print_mat(ma1);
    std::println("e0: {}, e1: {}", ma1.extent(0), ma1.extent(1));
    auto ma2 = std::mdspan{x.data(), 4, 2};
    print_mat(ma2);
    std::println("e0: {}, e1: {}", ma2.extent(0), ma2.extent(1));
}
