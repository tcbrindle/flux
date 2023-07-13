
// Copyright (c) 2015-2017 Bryce Adelstein Lelbach
// Copyright (c) 2020-2023 Corentin Jabot
// Copyright (c) 2017-2023 NVIDIA Corporation (reply-to: brycelelbach@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <ranges>
#include <tuple>

namespace std::ranges {

namespace detail {

template <typename First, typename... R>
constexpr bool valid_cartesian_product_pack
  = std::ranges::input_range<First> && (std::ranges::forward_range<R> && ...);

template <class R>
concept cartesian_product_simple_view
  = std::ranges::view<R> &&std::ranges::range<const R>
 && std::same_as<std::ranges::iterator_t<R>, std::ranges::iterator_t<const R>>
 && std::same_as<std::ranges::sentinel_t<R>, std::ranges::sentinel_t<const R>>;

} // namespace detail

template <std::ranges::view... Ranges>
  requires(sizeof...(Ranges) == 0) || detail::valid_cartesian_product_pack<Ranges...>
struct cartesian_product_view
  : std::ranges::view_interface<cartesian_product_view<Ranges...>>
{
private:
  std::tuple<Ranges...> bases;

public:
  constexpr cartesian_product_view() = default;

  constexpr cartesian_product_view(Ranges... base_)
    : bases(std::move(base_)...) {}

  template <bool IsConst>
  struct sentinel;

  template <bool IsConst>
  struct iterator
  {
  private:
    using parent = std::conditional_t<
      IsConst, cartesian_product_view const, cartesian_product_view
    >;

    parent* view = nullptr;
    std::tuple<std::ranges::iterator_t<Ranges>...> its;

    template <bool UIsConst>
    friend struct cartesian_product_view::sentinel;

    static constexpr auto iterator_category_impl() {
      if constexpr ((std::ranges::random_access_range<Ranges> && ...))
        return std::random_access_iterator_tag{};
      else if constexpr ((std::ranges::bidirectional_range<Ranges> && ...))
        return std::bidirectional_iterator_tag{};
      else if constexpr ((std::ranges::forward_range<Ranges> && ...))
        return std::forward_iterator_tag{};
      else if constexpr ((std::ranges::input_range<Ranges> && ...))
        return std::input_iterator_tag{};
      else
        return std::output_iterator_tag{};
    }

  public:
    using iterator_category = decltype(iterator_category_impl());
    using reference = std::tuple<std::ranges::range_reference_t<Ranges>...>;
    using value_type = std::tuple<std::ranges::range_value_t<Ranges>...>;
    using difference_type = std::common_type_t<std::ranges::range_difference_t<Ranges>...>;

    constexpr iterator() = default;
    constexpr explicit iterator(
      parent* view_, std::ranges::iterator_t<Ranges>... its_
    )
      : view(view_), its(std::move(its_)...) {}

    constexpr auto operator*() const
    {
      return std::apply(
        [&](auto const&... args) { return reference{*(args)...}; }
      , its
      );
    }

    constexpr iterator operator++(int)
    {
      if constexpr ((std::ranges::forward_range<Ranges> && ...)) {
        auto tmp = *this;
        ++*this;
        return tmp;
      }
      ++*this;
    }

    constexpr iterator& operator++()
    {
      next();
      return *this;
    }

    constexpr iterator& operator--()
      requires(std::ranges::bidirectional_range<Ranges> && ...)
    {
      prev();
      return *this;
    }

    constexpr iterator operator--(int)
      requires(std::ranges::bidirectional_range<Ranges> && ...)
    {
      auto tmp = *this;
      --*this;
      return tmp;
    }

    constexpr iterator& operator+=(difference_type n)
      requires(std::ranges::random_access_range<Ranges> && ...)
    {
      advance(n);
      return *this;
    }

    constexpr iterator &operator-=(difference_type n)
      requires(std::ranges::random_access_range<Ranges> && ...)
    {
      advance(-n);
      return *this;
    }

    friend constexpr iterator operator+(iterator i, difference_type n)
      requires(std::ranges::random_access_range<Ranges> && ...)
    {
      return {i + n};
    }

    friend constexpr iterator operator+(difference_type n, iterator i)
      requires(std::ranges::random_access_range<Ranges> && ...)
    {
      return {i + n};
    }

    friend constexpr iterator operator-(iterator i, difference_type n)
      requires(std::ranges::random_access_range<Ranges> && ...)
    {
      return {i - n};
    }

    friend constexpr difference_type operator-(
      iterator const& x, iterator const& y
    )
      requires(std::ranges::random_access_range<Ranges> && ...)
    {
      return y.distance(x);
    }

    constexpr decltype(auto) operator[](difference_type n) const
      requires(std::ranges::random_access_range<Ranges> && ...)
    {
      return *iterator{*this + n};
    }

    constexpr bool operator==(iterator const& other) const
    {
      if (at_end() && other.at_end())
        return true;
      return eq(*this, other);
    }

    friend constexpr auto operator<=>(iterator const& x, iterator const& y)
      requires(
        (std::ranges::random_access_range<Ranges> && ...) &&
        (std::three_way_comparable<std::ranges::iterator_t<Ranges>> && ...)
      )
    {
      return compare(x, y);
    }

    friend constexpr bool operator==(const iterator &i, sentinel<IsConst> const&)
    {
      return i.at_end();
    }
    friend constexpr bool operator==(const iterator &i, sentinel<!IsConst> const&)
    {
      return i.at_end();
    }

  private:
    constexpr bool at_end() const
    {
      auto const& v = std::get<0>(view->bases);
      return std::end(v) == std::get<0>(its);
    }

    template <auto N = 0>
    constexpr static auto compare(iterator const& a, iterator const& b)
      -> std::strong_ordering
    {
      auto cmp = std::get<N>(a.its) <=> std::get<N>(b.its);
      if constexpr (N + 1 < sizeof...(Ranges)) {
        if (cmp == 0)
          return compare<N + 1>(a, b);
      }
      return cmp;
    }

    template <auto N = sizeof...(Ranges) - 1>
    constexpr static bool eq(iterator const& a, iterator const& b)
    {
      if (std::get<N>(a.its) != std::get<N>(b.its))
        return false;
      if constexpr (N > 0)
        return eq<N - 1>(a, b);
      return true;
    }

    template <auto N = sizeof...(Ranges) - 1>
    constexpr void next()
    {
      const auto &v = std::get<N>(view->bases);
      auto &it = std::get<N>(its);
      if (++it == std::end(v)) {
        if constexpr (N != 0) {
          it = std::ranges::begin(v);
          next<N - 1>();
        }
      }
    }

    template <auto N = sizeof...(Ranges) - 1>
    constexpr void prev()
    {
      const auto &v = std::get<N>(view->bases);
      auto &it = std::get<N>(its);
      if (it == std::ranges::begin(v))
      {
        std::ranges::advance(it, std::ranges::end(v));
        if constexpr (N > 0)
          prev<N - 1>();
      }
      --it;
    }

    template <std::size_t N = sizeof...(Ranges) - 1>
    constexpr difference_type distance(iterator const& other) const
    {
      if constexpr (N == 0) {
        return std::get<0>(other.its) - std::get<0>(its);
      } else {
        const auto d = this->distance<N - 1>(other);
        auto const scale = std::ranges::distance(std::get<N>(view->bases));
        auto const increment = std::get<N>(other.its) - std::get<N>(its);

        return difference_type{(d * scale) + increment};
      }
    }

    template <std::size_t N = sizeof...(Ranges) - 1>
    void advance(difference_type n)
    {
      if (n == 0)
        return;

      auto &i = std::get<N>(its);
      auto const size = static_cast<difference_type>(
        std::ranges::size(std::get<N>(view->bases))
      );
      auto const first = std::ranges::begin(std::get<N>(view->bases));

      auto const idx = static_cast<difference_type>(i - first);
      n += idx;

      auto div = size ? n / size : 0;
      auto mod = size ? n % size : 0;

      if constexpr (N != 0) {
        if (mod < 0) {
          mod += size;
          div--;
        }
        advance<N - 1>(div);
      } else {
        if (div > 0) {
          mod = size;
        }
      }
      using D = std::iter_difference_t<decltype(first)>;
      i = first + static_cast<D>(mod);
    }
  };

  template <bool IsConst>
  struct sentinel
  {
  private:
    friend iterator<false>;
    friend iterator<true>;

    using parent = std::conditional_t<
      IsConst, cartesian_product_view const, cartesian_product_view
    >;

    parent* view = nullptr;
    std::tuple<std::ranges::sentinel_t<Ranges>...> end;

  public:
    sentinel() = default;

    constexpr explicit sentinel(
      parent* view_, std::ranges::sentinel_t<Ranges>... end_
    )
      : view(view_), end(std::move(end_)...) {
    }

    constexpr sentinel(sentinel<!IsConst> other)
      requires(
          IsConst
       && (std::convertible_to<
             std::ranges::sentinel_t<Ranges>
           , std::ranges::sentinel_t<const Ranges>
           > && ...)
      )
      : view(other.view_), end(other.end_) {}
  };

public:
  constexpr auto size() const
    requires(std::ranges::sized_range<Ranges> && ...)
  {
    return std::apply(
      [] <typename... Args> (Args const&... args)
      {
        using Size = std::common_type_t<std::ranges::range_size_t<Args>...>;
        return (Size(std::ranges::size(args)) * ...);
      }
    , bases);
  }

  constexpr auto begin()
    requires(!detail::cartesian_product_simple_view<Ranges> || ...)
  {
    return std::apply(
      [&] (auto&... args)
      {
        using std::ranges::begin;
        return iterator<false>{this, begin(args)...};
      }
    , bases
    );
  }

  constexpr auto begin() const
    requires(detail::cartesian_product_simple_view<Ranges> && ...)
  {
    return std::apply(
      [&] (auto const&... args)
      {
        using std::ranges::begin;
        return iterator<true>{this, begin(args)...};
      }
    , bases
    );
  }

  constexpr auto end() const
    requires(std::ranges::common_range<Ranges> && ...)
  {
    return std::apply(
      [&] (auto const& first, auto const&... args)
      {
        using std::ranges::end;
        using std::ranges::begin;
        return iterator<true>(this, end(first), begin(args)...);
      },
      bases
    );
  }

  constexpr auto end() const
    requires(!std::ranges::common_range<Ranges> || ...)
  {
    return std::apply(
      [&] (auto const&... args) {
        return sentinel<true>(this, std::end(args)...);
      }
    , bases
    );
  }
};

template <typename... Ranges>
cartesian_product_view(Ranges&&...) ->
  cartesian_product_view<std::ranges::views::all_t<Ranges>...>;

namespace detail {

struct cartesian_product_fn
{
  template <typename... Ranges>
  constexpr auto operator()(Ranges&&... ranges) const
  {
    return cartesian_product_view((Ranges&&)ranges...);
  }
};

} // namespace detail

namespace views {

inline constexpr detail::cartesian_product_fn cartesian_product{};

} // namespace views

} // namespace std::ranges

