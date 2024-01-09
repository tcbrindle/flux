
// Copyright (c) 2023 Jiri Nytra (jiri.nytra at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <flux.hpp>

#include <cassert>
#include <cstddef>
#include <iostream>
#include <iterator>
#include <limits>
#include <optional>
#include <ostream>
#include <random>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

using namespace std::string_literals;

auto intersperse = [](flux::sequence auto seq, std::string sep) -> flux::generator<std::string>
{
    bool first = true;
    for (auto c: seq) {
        if (first) {
            co_yield c;
            first = false;
        } else {
            co_yield sep;
            co_yield c;
        }
    }
};

namespace color {
    std::string yellow(const std::string& s) { 
        return "\u001b[33m"s + s + "\u001b[37m"; 
    }
}

class maze {
private:
    constexpr static auto wall = std::numeric_limits<int>::max();
    constexpr static auto path = 0;
    std::size_t width_;
    std::size_t height_;
    std::vector<int> fields_;

    auto adjacent(std::size_t pos) const {
        auto pos_x = pos % width_;
        auto pos_y = pos / width_;
        std::vector<std::size_t> adj;

        if (pos_x > 0 && fields_[pos - 1] != wall) { adj.push_back(pos - 1); }
        if (pos_x < width_ - 1 && fields_[pos + 1] != wall) { adj.push_back(pos + 1); }
        if (pos_y > 0 && fields_[pos - width_] != wall) { adj.push_back(pos - width_); }
        if (pos_y < height_ - 1 && fields_[pos + width_] != wall) { adj.push_back(pos + width_); }

        return flux::from(std::move(adj));
    };

public:
    maze(std::size_t width = 10, std::size_t height = 10):
        width_(width),
        height_(height),
        fields_(width * height, 0)
    { 
        assert(width > 1 && height > 1);
    }

    static maze random(std::size_t width = 10, std::size_t height = 10) {
        auto seed = std::random_device{}();
        std::mt19937 rng(seed);
        std::uniform_int_distribution dist(1, 9);
        maze m(width, height);
        
        for (auto i: flux::ints(1, flux::size(m.fields_) - 1)) {
             m.fields_[size_t(i)] = (rng() % 4) == 0 ? wall : dist(rng);
        }

        return m;
    }

    void print(std::ostream& s, bool print_costs = true) const {
        auto to_char = [print_costs] (int num) {
            if (num == wall) { return "#"s; }
            if (num == path) { return color::yellow("*"); }
            return print_costs ? std::to_string(num) : " "s; 
        };
        auto width = width_ * 2 + 3;
        auto edge = flux::single("|"s);
        auto h_edge = "+"s + std::string(width - 2, '-') + "+\n"s;
        auto out = std::ostream_iterator<std::string>(s);
        
        s << h_edge;
        for (auto&& row: flux::ref(fields_).map(to_char).chunk(width_)) {
            intersperse(flux::chain(flux::ref(edge), FLUX_FWD(row), flux::ref(edge)), " "s).output_to(out);
            s << '\n';
        }
        s << h_edge;
    }

    void mark_shortest_path() {
        struct edge_t { std::size_t src, dst; };

        std::vector<flux::optional<int>> costs(fields_.size());
        std::vector<flux::optional<size_t>> prevs(fields_.size());

        auto valid = [this](std::size_t u) { 
            return fields_[u] != wall; 
        };

        auto to_adjacent_edges = [this](std::size_t u) {
            auto to_edge = [](auto e) { return edge_t{std::get<0>(e), std::get<1>(e)}; };
            return flux::cartesian_product(flux::single(u), adjacent(u)).map(to_edge) ;
        };
        
        bool updated = false;
        auto update_costs_and_prevs = [this, &costs, &prevs, &updated](edge_t e) {
            constexpr auto max = std::numeric_limits<int>::max();
            if (costs[e.src] && (*costs[e.src] + fields_[e.dst]) < costs[e.dst].value_or(max)) {
                costs[e.dst].emplace((*costs[e.src] + fields_[e.dst]));
                prevs[e.dst].emplace(e.src);
                updated = true;
            }
        };

        costs[0].emplace(0);
        do {
            updated = false;
            flux::iota(size_t{0}, fields_.size())
                .filter(valid) 
                .map(to_adjacent_edges)
                .flatten()
                .for_each(update_costs_and_prevs);
        } while(updated);

        for (auto pos = *flux::back(prevs); pos; pos = prevs[*pos]) {
            fields_[*pos] = path;
        }
    }
};

void print_side_by_side(std::istream& s1, std::istream& s2) {
    for (auto [line1, line2]: flux::zip(flux::getlines(s1, '\n'), flux::getlines(s2, '\n'))) {
        std::cout << line1 << "  " << line2 << '\n';
    }
}

int main() {
    auto maze = maze::random();

    std::stringstream ss1;
    maze.print(ss1);

    maze.mark_shortest_path();

    std::stringstream ss2;
    maze.print(ss2, false);

    print_side_by_side(ss1, ss2);
}
