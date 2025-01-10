#pragma once

#include <concepts>
#include <vector>

namespace flux::detail {

// Calculates the factorial of `x`
[[nodiscard]] constexpr auto factorial(const std::integral auto x) -> decltype(x)
{
    if (x <= 1) {
        return 1;
    }
    return x * factorial(x - 1);
}

// Given an input vector and a range of indices, return a new vector with the same values
// of `input`, ordered by `indices` up to the given `length`.
template <typename T>
[[nodiscard]] constexpr auto reindex_vec(const std::vector<T>& input, const auto& indices,
                                         const std::size_t length) -> std::vector<T>
{
    std::vector<T> output;
    output.reserve(input.size());

    for (std::size_t i = 0; i < length; i++) {
        output.push_back(input[indices[i]]);
    }

    return output;
}

// Given an input vector and a range of indices, return a new vector with the same values
// of `input`, ordered by `indices`.
template <typename T>
[[nodiscard]] constexpr auto reindex_vec(const std::vector<T>& input, const auto& indices)
    -> std::vector<T>
{
    return reindex_vec(input, indices, input.size());
}
} // namespace flux::detail
