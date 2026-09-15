
module;

#include <flux.hpp>

export module flux;

export namespace flux {

// adaptor/adjacent_filter.hpp
using flux::adjacent_filter;
using flux::dedup;

// adaptor/adjacent.hpp
using flux::adjacent;
using flux::adjacent_map;
using flux::pairwise;
using flux::pairwise_map;

// adaptor/cache_last.hpp
using flux::cache_last;

// adaptor/cartesian_base.hpp

// adaptor/cartesian_power_map.hpp
using flux::cartesian_power_map;

// adaptor/cartesian_power.hpp
using flux::cartesian_power;

// adaptor/cartesian_product_map.hpp
using flux::cartesian_product_map;

// adaptor/cartesian_product.hpp
using flux::cartesian_product;

// adaptor/chain.hpp
using flux::chain;

// adaptor/chunk_by.hpp
using flux::chunk_by;

// adaptor/chunk.hpp
using flux::chunk;

// adaptor/cursors.hpp
using flux::cursors;

// adaptor/cycle.hpp
using flux::cycle;

// adaptor/drop_while.hpp
using flux::drop_while;

// adaptor/drop.hpp
using flux::drop;

// adaptor/filter_map.hpp
using flux::filter_deref;
using flux::filter_map;

// adaptor/filter.hpp
using flux::filter;

// adaptor/flatten_with.hpp
using flux::flatten_with;

// adaptor/flatten.hpp
using flux::flatten;

// adaptor/map.hpp
using flux::map;

// adaptor/mask.hpp
using flux::mask;

// adaptor/read_only.hpp
using flux::read_only;

// adaptor/reverse.hpp
using flux::reverse;

// adaptor/scan_first.hpp
using flux::scan_first;

// adaptor/scan.hpp
using flux::prescan;
using flux::scan;

// adaptor/set_adaptors.hpp
using flux::set_difference;
using flux::set_intersection;
using flux::set_symmetric_difference;
using flux::set_union;

// adaptor/slide.hpp
using flux::slide;

// adaptor/split_string.hpp
using flux::split_string;

// adaptor/split.hpp
using flux::split;

// adaptor/stride.hpp
using flux::stride;

// adaptor/take_while.hpp
using flux::take_while;

// adaptor/take.hpp
using flux::take;

// adaptor/unchecked.hpp
using flux::unchecked;

// adaptor/zip.hpp
using flux::zip;
using flux::zip_map;

// algorithm/all_any_none.hpp
using flux::all;
using flux::any;
using flux::none;

// algorithm/compare.hpp
using flux::compare;

// algorithm/contains.hpp
using flux::contains;

// algorithm/count.hpp
using flux::count;
using flux::count_eq;
using flux::count_if;

// algorithm/ends_with.hpp
using flux::ends_with;

// algorithm/equal.hpp
using flux::equal;

// algorithm/fill.hpp
using flux::fill;

// algorithm/find_min_max.hpp
using flux::find_max;
using flux::find_min;
using flux::find_minmax;

// algorithm/find.hpp
using flux::find;
using flux::find_if;
using flux::find_if_not;

// algorithm/fold.hpp
using flux::fold;
using flux::fold_first;
using flux::product;
using flux::sum;

// algorithm/for_each.hpp
using flux::for_each;

// algorithm/inplace_reverse.hpp
using flux::inplace_reverse;

// algorithm/minmax.hpp
using flux::max;
using flux::min;
using flux::minmax;
using flux::minmax_result;

// algorithm/output_to.hpp
using flux::output_to;

// algorithm/search.hpp
using flux::search;

// algorithm/sort.hpp
using flux::sort;

// algorithm/starts_with.hpp
using flux::starts_with;

// algorithm/swap_elements.hpp
using flux::swap_elements;

// algorithm/to.hpp
using flux::from_sequence;
using flux::from_sequence_t;
using flux::to;

// algorithm/write_to.hpp
using flux::write_to;

// algorithm/zip_algorithms.hpp
using flux::zip_find_if;
using flux::zip_fold;
using flux::zip_for_each;
using flux::zip_for_each_while;

// core/assert.hpp
using flux::assert_;
using flux::bounds_check;
using flux::indexed_bounds_check;
using flux::runtime_error;
using flux::unrecoverable_error;

// core/concepts.hpp
using flux::adaptable_sequence;
using flux::bidirectional_sequence;
using flux::bounded_sequence;
using flux::common_element_t;
using flux::const_element_t;
using flux::const_iterable_sequence;
using flux::contiguous_sequence;
using flux::cursor;
using flux::cursor_t;
using flux::default_sequence_traits;
using flux::distance_t;
using flux::element_t;
using flux::index_t;
using flux::infinite_sequence;
using flux::inline_sequence_base;
using flux::multipass_sequence;
using flux::ordered_cursor;
using flux::random_access_sequence;
using flux::read_only_sequence;
using flux::regular_cursor;
using flux::rvalue_element_t;
using flux::sequence;
using flux::sequence_traits;
using flux::sized_sequence;
using flux::value_t;
using flux::writable_sequence_of;

// core/config.hpp
using flux::divide_by_zero_policy;
using flux::error_policy;
using flux::integer_cast_policy;
using flux::overflow_policy;
namespace config {
using flux::config::enable_debug_asserts;
using flux::config::int_type;
using flux::config::on_divide_by_zero;
using flux::config::on_error;
using flux::config::on_integer_cast;
using flux::config::on_overflow;
using flux::config::print_error_on_terminate;
} // namespace config

// core/default_impls.hpp

// core/functional.hpp
using flux::flip;
using flux::proj;
using flux::proj2;
using flux::unpack;
namespace pred {
using flux::pred::both;
using flux::pred::either;
using flux::pred::eq;
using flux::pred::even;
using flux::pred::false_;
using flux::pred::geq;
using flux::pred::gt;
using flux::pred::id;
using flux::pred::in;
using flux::pred::leq;
using flux::pred::lt;
using flux::pred::negative;
using flux::pred::neither;
using flux::pred::neq;
using flux::pred::nonzero;
using flux::pred::not_;
using flux::pred::odd;
using flux::pred::positive;
using flux::pred::true_;
namespace detail {
// FIXME: exports from detail namespace
using flux::pred::detail::operator!;
using flux::pred::detail::operator&&;
using flux::pred::detail::operator||;
} // namespace detail
} // namespace pred
namespace cmp {
using flux::cmp::compare;
using flux::cmp::compare_floating_point_unchecked;
using flux::cmp::max;
using flux::cmp::min;
using flux::cmp::partial_max;
using flux::cmp::partial_min;
using flux::cmp::reverse_compare;
} // namespace cmp

// core/inline_sequence_base.hpp
using flux::bounds;
using flux::bounds_t;

// core/numeric.hpp
namespace num {
using flux::num::cast;
using flux::num::checked_cast;
using flux::num::integral;
using flux::num::overflow_result;
using flux::num::overflowing_cast;
using flux::num::signed_integral;
using flux::num::unchecked_cast;
using flux::num::unsigned_integral;

using flux::num::unchecked_add;
using flux::num::unchecked_div;
using flux::num::unchecked_mod;
using flux::num::unchecked_mul;
using flux::num::unchecked_neg;
using flux::num::unchecked_shl;
using flux::num::unchecked_shr;
using flux::num::unchecked_sub;

using flux::num::wrapping_add;
using flux::num::wrapping_mul;
using flux::num::wrapping_neg;
using flux::num::wrapping_sub;

using flux::num::overflowing_add;
using flux::num::overflowing_mul;
using flux::num::overflowing_neg;
using flux::num::overflowing_sub;

using flux::num::checked_add;
using flux::num::checked_div;
using flux::num::checked_mod;
using flux::num::checked_mul;
using flux::num::checked_neg;
using flux::num::checked_shl;
using flux::num::checked_shr;
using flux::num::checked_sub;

using flux::num::add;
using flux::num::div;
using flux::num::mod;
using flux::num::mul;
using flux::num::neg;
using flux::num::shl;
using flux::num::shr;
using flux::num::sub;
} // namespace num

// core/operation_requirements.hpp
using flux::fold_result_t;
using flux::foldable;
using flux::weak_ordering_for;

// core/optional.hpp
using flux::nullopt;
using flux::nullopt_t;
using flux::optional;

// core/ref.hpp
using flux::from;
using flux::from_fwd_ref;
using flux::mut_ref;
using flux::ref;

// core/sequence_access.hpp
using flux::back;
using flux::data;
using flux::dec;
using flux::distance;
using flux::first;
using flux::for_each_while;
using flux::front;
using flux::inc;
using flux::is_empty;
using flux::is_last;
using flux::last;
using flux::move_at;
using flux::move_at_unchecked;
using flux::next;
using flux::prev;
using flux::read_at;
using flux::read_at_unchecked;
using flux::size;
using flux::swap_at;
using flux::swap_with;
using flux::usize;

// core/sequence_iterator.hpp
using flux::begin;
using flux::end;

// core/slice.hpp
using flux::slice;

// core/utils.hpp
using flux::copy;
using flux::decays_to;
using flux::ordering_invocable;
using flux::same_decayed;

// sequence/array_ptr.hpp
using flux::array_ptr;
using flux::make_array_ptr_unchecked;

// sequence/bitset.hpp

// sequence/empty.hpp
using flux::empty;

// sequence/generator.hpp
using flux::generator;

// sequence/getlines.hpp
using flux::getlines;

// sequence/iota.hpp
using flux::ints;
using flux::iota;

// sequence/istream.hpp
using flux::from_istream;

// sequence/istreambuf.hpp
using flux::from_istreambuf;

// sequence/range.hpp
using flux::from_crange;
using flux::from_range;

// sequence/repeat.hpp
using flux::repeat;

// sequence/single.hpp
using flux::single;

// sequence/unfold.hpp
using flux::unfold;

} // namespace flux
