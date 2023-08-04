
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_HPP_INCLUDED
#define FLUX_HPP_INCLUDED

#include <flux/core.hpp>

#include <flux/op/adjacent.hpp>
#include <flux/op/all_any_none.hpp>
#include <flux/op/begin_end.hpp>
#include <flux/op/cache_last.hpp>
#include <flux/op/cartesian_product.hpp>
#include <flux/op/cartesian_product_with.hpp>
#include <flux/op/chain.hpp>
#include <flux/op/chunk.hpp>
#include <flux/op/chunk_by.hpp>
#include <flux/op/compare.hpp>
#include <flux/op/contains.hpp>
#include <flux/op/count.hpp>
#include <flux/op/cursors.hpp>
#include <flux/op/cycle.hpp>
#include <flux/op/drop.hpp>
#include <flux/op/drop_while.hpp>
#include <flux/op/ends_with.hpp>
#include <flux/op/equal.hpp>
#include <flux/op/fill.hpp>
#include <flux/op/filter.hpp>
#include <flux/op/find.hpp>
#include <flux/op/find_min_max.hpp>
#include <flux/op/flatten.hpp>
#include <flux/op/fold.hpp>
#include <flux/op/for_each.hpp>
#include <flux/op/for_each_while.hpp>
#include <flux/op/from.hpp>
#include <flux/op/inplace_reverse.hpp>
#include <flux/op/map.hpp>
#include <flux/op/mask.hpp>
#include <flux/op/minmax.hpp>
#include <flux/op/read_only.hpp>
#include <flux/op/ref.hpp>
#include <flux/op/reverse.hpp>
#include <flux/op/scan.hpp>
#include <flux/op/scan_first.hpp>
#include <flux/op/set_adaptors.hpp>
#include <flux/op/slice.hpp>
#include <flux/op/slide.hpp>
#include <flux/op/sort.hpp>
#include <flux/op/split.hpp>
#include <flux/op/split_string.hpp>
#include <flux/op/starts_with.hpp>
#include <flux/op/stride.hpp>
#include <flux/op/swap_elements.hpp>
#include <flux/op/take.hpp>
#include <flux/op/take_while.hpp>
#include <flux/op/to.hpp>
#include <flux/op/unchecked.hpp>
#include <flux/op/write_to.hpp>
#include <flux/op/zip.hpp>
#include <flux/op/zip_algorithms.hpp>

#include <flux/source/array_ptr.hpp>
#include <flux/source/empty.hpp>
#include <flux/source/generator.hpp>
#include <flux/source/getlines.hpp>
#include <flux/source/iota.hpp>
#include <flux/source/istream.hpp>
#include <flux/source/istreambuf.hpp>
#include <flux/source/range.hpp>
#include <flux/source/repeat.hpp>
#include <flux/source/single.hpp>
#include <flux/source/unfold.hpp>

#endif
