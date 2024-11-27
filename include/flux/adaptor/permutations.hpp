
// Copyright stuff here? Not really sure on that as I've never done
// it

#ifndef FLUX_ADAPTOR_PERMUTATIONS_HPP_INCLUDED
#define FLUX_ADAPTOR_PERMUTATIONS_HPP_INCLUDED

#include "flux/core/concepts.hpp"
#include "flux/core/inline_sequence_base.hpp"
#include "flux/macros.hpp"
#include <cstddef>
#include <flux/core.hpp>

namespace flux {

namespace detail {

template <flux::sequence Base, std::size_t Length>
    requires(Length > 0)
    && flux::bounded_sequence<Base> // permutation of size 0 don't make sense and infinite sequences
                                    // don't make sense either
                                    struct permutations_adaptor
    : inline_sequence_base<permutations_adaptor<Base, Length>> {
private:
    Base base_;
    inline static constexpr std::size_t length_
        = Length; // set this as a member variable for now in case we want to make this a runtime
                  // parameter in the future instead of a template parameter

public:
    constexpr permutations_adaptor(Base&& base) : base_(FLUX_FWD(base)) { }

    struct flux_sequence_traits : default_sequence_traits {
    private:
        using self_t = permutations_adaptor;
        using element_t = std::vector<flux::value_t<Base>>;

        struct cursor_type {
            /* final type is todo */ std::vector<flux::value_t<Base>> vals_;
            /* final type is todo */ std::vector<flux::cursor_t<Base>> indices;
            /* final type is todo */ std::vector<flux::cursor_t<Base>> cycles;

            constexpr bool operator==(cursor_type const&) const = default;
        };

    public:
        using value_type = std::vector<flux::value_t<Base>>;
    };
};
} // namespace detail

} // namespace flux
#endif
