
// Copyright (c) 2023 NVIDIA Corporation (reply-to: brycelelbach@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <flux/op/cartesian_product.hpp>
#include <flux/source/iota.hpp>
#include <flux/op/for_each.hpp>

void memset_2d_reference(double* A, std::size_t N, std::size_t M)
{
    for (std::size_t j = 0; j != M; ++j)
        for (std::size_t i = 0; i != N; ++i)
            A[i + j * N] = 0.0;
}

void memset_2d_flux_cartesian_product_iota(double* A, std::size_t N, std::size_t M)
{
    flux::cartesian_product(flux::iota(0LU, N), flux::iota(0LU, M))
        .for_each(flux::unpack([&] (auto i, auto j) {
            A[i + j * N] = 0.0;
        }));
}
