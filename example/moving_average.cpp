// Copyright (c) 2023 Jiri Nytra (jiri.nytra at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <flux.hpp>

#include <cstddef>
#include <cassert>
#include <deque>
#include <vector>

struct sliding_window_t
{
    explicit sliding_window_t(std::size_t N): size(N) {
    }

    void push(int value) {
        if (window.size() < size) {
            sum += value;
        } else {
            sum += value - window.front();
            window.pop_front();
        }

        window.push_back(value);
    }

    int average() const {
        return sum / window.size();
    }

    std::size_t size;
    std::deque<int> window;
    int sum = 0;
};

auto sliding_window = [](sliding_window_t&& win, int next) {
    win.push(next);
    return std::move(win);
};

int main() {
    std::vector intervals = {1, 5, 6, 1, 2, 9, 7, -1, 0};

    // compute moving average by scan adaptor (more effective for large windows)
    auto ma = flux::from(intervals)
        .scan(sliding_window, sliding_window_t(3))
        .map(flux::proj(&sliding_window_t::average))
        .to<std::vector>();

    assert(ma.size() == intervals.size());
    assert(ma[0] == 1);
    assert(ma[1] == 3); // (1 + 5) / 2
    assert(ma[2] == 4); // (1 + 5 + 6) / 3
    assert(ma[3] == 4); // (5 + 6 + 1) / 3
    assert(ma.back() == 2); // (7 + -1 + 0) / 3

    // compute moving average by slide adaptor (less effective for large windows)
    auto ma2 = flux::from(intervals)
        .slide(3)
        .map([](auto&& win) { return win.sum() / win.size(); })
        .to<std::vector>();

    assert(ma2.size() == intervals.size() - 2);
    assert(ma2[0] == 4); // (1 + 5 + 6) / 3
    assert(ma2[1] == 4); // (5 + 6 + 1) / 3
    assert(ma2.back() == 2); // (7 + -1 + 0) / 3
}