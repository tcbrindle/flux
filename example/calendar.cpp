
// Copyright (c) 2023 Jiri Nytra (jiri.nytra at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)


#include <flux.hpp>

#include <algorithm>
#include <charconv>
#include <map>
#include <chrono>
#include <cstddef>
#include <functional>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <utility>

using namespace std::chrono;
using namespace std::string_literals;
using ymd = year_month_day;

constexpr auto day_pad_size = 1UL;
constexpr auto day_str_size = day_pad_size + 2UL + day_pad_size;
constexpr auto week_str_size = 7UL * day_str_size;
constexpr auto max_weeks_in_month = 6;
constexpr auto col_sep = "  ";
constexpr auto row_sep = ' ';

// Workaround: libc++17 does not support C++20 chrono::time_point::operator++
#if defined _LIBCPP_VERSION and _LIBCPP_VERSION < 180000
namespace std::chrono {
    sys_days& operator++(sys_days& d) {
        return d += days{1};
    }

    sys_days operator++(sys_days& d, int) {
        return std::exchange(d, d + days{1});
    }
}
#endif

auto dates(ymd from, ymd to) {
    return flux::iota(sys_days{from}, sys_days{to});
}

auto month_num = [](sys_days d1, sys_days d2) { 
    return ymd(d1).month() == ymd(d2).month(); 
};

auto week_num = [](sys_days d1, sys_days d2) { 
    return weekday(d1).iso_encoding() < weekday(d2).iso_encoding();
};

auto day_to_string = [](sys_days d) { 
    auto day_num = static_cast<unsigned>(ymd(d).day());
    auto day_str = (day_num < 10 ? " "s : ""s) + std::to_string(day_num);
    std::string pad_str(day_pad_size, ' ');
    return pad_str + day_str + pad_str;
};

auto month_name(sys_days d) { 
    constexpr std::array months = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", 
                                   "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    auto year = std::to_string(static_cast<int>(ymd{d}.year()));
    auto month = std::string{months[static_cast<unsigned>(ymd{d}.month()) - 1]};
    auto title = month + '-' + year;
    auto pad_size = (week_str_size - title.size()) / 2;
    auto aligned_title = (std::string(pad_size, ' ') + title);
    aligned_title.resize(week_str_size, ' ');
    return aligned_title;
};

// In: sequence<sys_day> (week days)
// Out: std::string (week string)
auto week_to_string = [](flux::sequence auto&& week) { 
    bool first_week_in_month = ymd(*flux::front(week)).day() == 1d;
    auto week_str = FLUX_FWD(week)
                     .map(day_to_string)
                     .flatten()
                     .template to<std::string>();                     
    if (week_str.size() < week_str_size) {
        std::string padding(week_str_size - week_str.size(), ' ');
        week_str = first_week_in_month ? padding + week_str : week_str + padding;
    }

    return week_str;
};

// In: sequence<sys_day> (month days)
// Out: sequence<std::string> (month calendar)
auto to_week_lines = [](flux::sequence auto&& month) { 
    auto month_str = month_name(*flux::front(month));
    auto week_lines = FLUX_FWD(month)
                        .chunk_by(week_num)
                        .map(week_to_string);
    std::vector<std::string> pad_lines(size_t(max_weeks_in_month - flux::count(week_lines)),
                                       std::string(week_str_size, ' '));
    return flux::chain(flux::single(std::move(month_str)), 
                       std::move(week_lines), 
                       std::move(pad_lines),
                       flux::single(std::string(week_str_size, row_sep)));
};

auto append_column = [](std::vector<std::string>&& months_per_line, flux::sequence auto&& month) {
    return flux::zip(std::move(months_per_line), FLUX_FWD(month).template to<std::vector>())
        .map([](const auto& v) { return v.first + v.second + col_sep; })
        .template to<std::vector>();
};

// In: sequence<sequence<std::string>> (chunks of months per line)
// Out: std::vector<std::string> (months organized in columns)
auto to_columns = [](flux::sequence auto&& month_chunk) {
    auto n_rows = month_chunk.front()->count();
    return FLUX_FWD(month_chunk).fold(append_column,
                                      std::vector<std::string>(size_t(n_rows), col_sep));
};

const auto current_year = ymd{floor<days>(system_clock::now())}.year();

struct app_args_t {
    int per_line = 3;
    ymd from = current_year / January / 1d;
    ymd to = from + years{1};
};

[[noreturn]] void print_help_and_exit(std::string_view app_name) {
    std::cout << "Usage: " << app_name << " [--help] [--per-line=num] [--from=year] [--to=year]\n\n"; 
    exit(0);
}

app_args_t parse_args(int argc, char** argv) {
    app_args_t result;
    std::string_view app_name = argv[0];
    std::vector<std::string> args(std::next(argv), std::next(argv, argc));

    using parse_func_t = std::function<void(std::string)>;
    std::map<std::string, parse_func_t> args_map {
        {"--help"s, [&]([[maybe_unused]] std::string) { print_help_and_exit(app_name); }},
        {"--per-line"s, [&](std::string val) { result.per_line = std::max(1, std::stoi(val)); }},
        {"--from"s, [&](std::string val) { 
            result.from = year{std::max(1, std::stoi(val))} / January / 1d; 
            result.to = (result.from.year() + years{1}) / January / 1d;}},
        {"--to"s, [&](std::string val) { result.to = ymd{year{std::max(1, std::stoi(val))}, January, 1d}; }}
    };

    auto to_pair = [](const auto& s) {
        auto pos = flux::find(flux::ref(s), '=');
        return std::pair{s.substr(0, size_t(pos)),
                         s.substr(size_t(std::min(flux::count(s), pos + 1)))};
    };

    for (const auto& [key, val] : flux::ref(args).map(to_pair)) {
        if (auto it = args_map.find(key); it != args_map.end()) {
            std::invoke(it->second, val);
        } else {
            throw std::runtime_error("Unknown option "s + key + ". Use --help for more info.");
        }
    }
    
    return result;
}

int main(int argc, char** argv)
try {
    auto args = parse_args(argc, argv);
    dates(args.from, args.to)
        .chunk_by(month_num)
        .map(to_week_lines)
        .chunk(args.per_line)
        .map(to_columns)
        .flatten()
        .output_to(std::ostream_iterator<std::string>(std::cout, "\n"));
}
catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return 1;
}