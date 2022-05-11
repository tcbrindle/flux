
#ifndef FLUX_OP_FOR_EACH_WHILE_HPP_INCLUDED
#define FLUX_OP_FOR_EACH_WHILE_HPP_INCLUDED

#include <flux/core/concepts.hpp>

namespace flux {

namespace detail {

struct for_each_while_fn {
    template <sequence Seq, typename Pred>
        requires std::invocable<Pred&, element_t<Seq>> &&
                 std::convertible_to<bool, std::invoke_result_t<Pred&, element_t<Seq>>>
    constexpr auto operator()(Seq&& seq, Pred pred) const -> index_t<Seq>
    {
        if constexpr (requires { iface_t<Seq>::for_each_while(seq, std::move(pred)); }) {
            return iface_t<Seq>::for_each_while(seq, std::move(pred));
        } else {
            auto idx = first(seq);
            while (!is_last(seq, idx)) {
                if (!std::invoke(pred, read_at(seq, idx))) { break; }
                inc(seq, idx);
            }
            return idx;
        }
    }
};

} // namespace detail

inline constexpr auto for_each_while = detail::for_each_while_fn{};

} // namespace flux

#endif // FLUX_OP_FOR_EACH_WHILE_HPP_INCLUDED
