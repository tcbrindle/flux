
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_HPP_INCLUDED
#define FLUX_HPP_INCLUDED

#include <flux/core.hpp>

#include <flux/op/all_any_none.hpp>
#include <flux/op/bounds_checked.hpp>
#include <flux/op/cache_last.hpp>
#include <flux/op/cartesian_product.hpp>
#include <flux/op/cartesian_product_with.hpp>
#include <flux/op/chain.hpp>
#include <flux/op/count.hpp>
#include <flux/op/drop.hpp>
#include <flux/op/drop_while.hpp>
#include <flux/op/equal.hpp>
#include <flux/op/fill.hpp>
#include <flux/op/filter.hpp>
#include <flux/op/find.hpp>
#include <flux/op/fold.hpp>
#include <flux/op/for_each.hpp>
#include <flux/op/for_each_while.hpp>
#include <flux/op/from.hpp>
#include <flux/op/inplace_reverse.hpp>
#include <flux/op/map.hpp>
#include <flux/op/minmax.hpp>
#include <flux/op/ref.hpp>
#include <flux/op/reverse.hpp>
#include <flux/op/slice.hpp>
#include <flux/op/split.hpp>
#include <flux/op/split_string.hpp>
#include <flux/op/sort.hpp>
#include <flux/op/swap_elements.hpp>
#include <flux/op/take.hpp>
#include <flux/op/take_while.hpp>
#include <flux/op/to.hpp>
#include <flux/op/write_to.hpp>
#include <flux/op/zip.hpp>

#include <flux/source/empty.hpp>
#include <flux/source/getlines.hpp>
#include <flux/source/iota.hpp>
#include <flux/source/istream.hpp>
#include <flux/source/istreambuf.hpp>
#include <flux/source/single.hpp>

#include <flux/ranges.hpp>

#endif
