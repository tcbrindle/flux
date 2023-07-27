
module;

#include <flux.hpp>

export module flux;

export namespace flux {
    // from core/assert.hpp
    using flux::unrecoverable_error;
    using flux::runtime_error;
    using flux::assert_;
    using flux::bounds_check;

    // from core/concepts.hpp
    using flux::cursor;
    using flux::regular_cursor;
    using flux::ordered_cursor;
    using flux::sequence_traits;
    using flux::cursor_t;
    using flux::element_t;
    using flux::value_t;
    using flux::distance_t;
    using flux::index_t;
    using flux::rvalue_element_t;
    using flux::common_element_t;
    using flux::const_element_t;
    using flux::sequence;
    using flux::multipass_sequence;
    using flux::bidirectional_sequence;
    using flux::random_access_sequence;
    using flux::contiguous_sequence;
    using flux::bounded_sequence;
    using flux::sized_sequence;
    using flux::writable_sequence_of;
    using flux::infinite_sequence;

    // from core/config.hpp
    using flux::error_policy;
    using flux::overflow_policy;
    namespace config {
        using flux::config::on_error;
        using flux::config::on_overflow;
        using flux::config::print_error_on_terminate;
        using flux::config::enable_debug_asserts;
    }

    // from core/default_impls.hpp
    /* no exports */

    // from core/functional.hpp
    using flux::proj;
    using flux::proj2;
    using flux::unpack;
    namespace pred {
        using flux::pred::not_;
        using flux::pred::both;
        using flux::pred::either;
        using flux::pred::neither;
        using flux::pred::eq;
        using flux::pred::neq;
        using flux::pred::lt;
        using flux::pred::gt;
        using flux::pred::leq;
        using flux::pred::geq;
        using flux::pred::true_;
        using flux::pred::false_;
        using flux::pred::id;
        using flux::pred::positive;
        using flux::pred::negative;
        using flux::pred::nonzero;
        using flux::pred::in;
        using flux::pred::even;
        using flux::pred::odd;
    }

    // from core/inline_sequence_base.hpp
    using flux::bounds;
    using flux::bounds_t;
    using flux::inline_sequence_base;

    // from core/macros.hpp
    /* no exports */

    // from core/numeric.hpp
    namespace num {
        using flux::num::wrapping_add;
        using flux::num::wrapping_sub;
        using flux::num::wrapping_mul;
        using flux::num::overflow_result;
        using flux::num::overflowing_add;
        using flux::num::overflowing_sub;
        using flux::num::overflowing_mul;
        using flux::num::checked_add;
        using flux::num::checked_sub;
        using flux::num::checked_mul;
    }

    // from core/optional.hpp
    using flux::nullopt_t;
    using flux::nullopt;
    using flux::optional;

    // from core/sequence_access.hpp
    using flux::first;
    using flux::is_last;
    using flux::read_at;
    using flux::move_at;
    using flux::read_at_unchecked;
    using flux::move_at_unchecked;
    using flux::inc;
    using flux::dec;
    using flux::distance;
    using flux::data;
    using flux::last;
    using flux::size;
    using flux::usize;
    using flux::next;
    using flux::prev;
    using flux::is_empty;
    using flux::swap_with;
    using flux::swap_at;
    using flux::front;
    using flux::back;

    // from core/simple_sequence_base.hpp
    using flux::simple_sequence_base;

    // from core/utils.hpp
    using flux::decays_to;
    using flux::copy;
    using flux::checked_cast;

    // from op/adjacent.hpp
    using flux::adjacent;
    using flux::pairwise;
    using flux::adjacent_map;
    using flux::pairwise_map;

    // from op/all_any_none.hpp
    using flux::all;
    using flux::none;
    using flux::any;

    // from op/begin_end.hpp
    using flux::begin;
    using flux::end;

    // from op/cache_last.hpp
    using flux::cache_last;

    // from op/cartesian_product.hpp
    using flux::cartesian_product;

    // from op/cartesian_product_with.hpp
    using flux::cartesian_product_with;

    // from op/chain.hpp
    using flux::chain;

    // from op/chunk.hpp
    using flux::chunk;

    // from op/chunk_by.hpp
    using flux::chunk_by;

    // from op/compare.hpp
    using flux::compare;

    // from op/contains.hpp
    using flux::contains;

    // from op/count.hpp
    using flux::count;
    using flux::count_eq;
    using flux::count_if;

    // from op/cursors.hpp
    using flux::cursors;

    // from op/cycle.hpp
    using flux::cycle;

    // from op/drop.hpp
    using flux::drop;

    // from op/drop_while.hpp
    using flux::drop_while;

    // from op/ends_with.hpp
    using flux::ends_with;

    // from op/equal.hpp
    using flux::equal;

    // from op/fill.hpp
    using flux::fill;

    // from op/filter.hpp
    using flux::filter;

    // from op/find.hpp
    using flux::find;
    using flux::find_if;
    using flux::find_if_not;

    // from op/flatten.hpp
    using flux::flatten;

    // from op/fold.hpp
    using flux::fold;
    using flux::fold_first;
    using flux::sum;
    using flux::product;

    // from op/for_each.hpp
    using flux::for_each;

    // from op/for_each_while.hpp
    using flux::for_each_while;

    // from op/from.hpp
    using flux::from;
    using flux::from_fwd_ref;

    // from op/inplace_reverse.hpp
    using flux::inplace_reverse;

    // from op/map.hpp
    using flux::map;

    // from op/mask.hpp
    using flux::mask;

    // from op/minmax.hpp
    using flux::minmax_result;
    using flux::min;
    using flux::max;
    using flux::minmax;

    // from op/output_to.hpp
    using flux::output_to;

    // from op/read_only.hpp
    using flux::read_only;

    // from op/ref.hpp
    using flux::mut_ref;
    using flux::ref;

    // from op/requirements.hpp
    using flux::fold_result_t;
    using flux::foldable;
    using flux::strict_weak_order_for;

    // from op/reverse.hpp
    using flux::reverse;

    // from op/scan.hpp
    using flux::scan;
    using flux::prescan;

    // from op/scan_first.hpp
    using flux::scan_first;

    // from op/search.hpp
    using flux::search;

    // from op/set_adaptors.hpp
    using flux::set_union;
    using flux::set_difference;
    using flux::set_symmetric_difference;
    using flux::set_intersection;

    // from op/slice.hpp
    using flux::slice;

    // from op/slide.hpp
    using flux::slide;

    // from op/sort.hpp
    using flux::sort;

    // from op/split.gpp
    using flux::split;

    // from op/split_string.hpp
    using flux::split_string;

    // from op/starts_with.hpp
    using flux::starts_with;

    // from op/stride.hpp
    using flux::stride;

    // from op/swap_elements.hpp
    using flux::swap_elements;

    // from op/take.hpp
    using flux::take;

    // from op/take_while.hpp
    using flux::take_while;

    // from op/to.hpp
    using flux::from_sequence_t;
    using flux::from_sequence;
    using flux::to;

    // from op/unchecked.hpp
    using flux::unchecked;

    // from op/write_to.hpp
    using flux::write_to;

    // from op/zip.hpp
    using flux::zip;

    // from op/zip_algorithms.hpp
    using flux::zip_for_each_while;
    using flux::zip_for_each;
    using flux::zip_find_if;
    using flux::zip_fold;


    // from source/array_ptr.hpp
    using flux::array_ptr;
    using flux::make_array_ptr_unchecked;

    // from source/bitset.hpp
    /* no exports */

    // from source/empty.hpp
    using flux::empty;

    // from source/generator.hpp
    using flux::generator;

    // from source/getlines.hpp
    using flux::getlines;

    // from source/iota.hpp
    using flux::iota;
    using flux::ints;

    // from source/istream.hpp
    using flux::from_istream;

    // from source/istreambuf.hpp
    using flux::from_istreambuf;

    // from source/range.hpp
    using flux::from_range;
    using flux::from_crange;

    // from source/repeat.hpp
    using flux::repeat;

    // from source/single.hpp
    using flux::single;

    // from source/unfold.hpp
    using flux::unfold;
}