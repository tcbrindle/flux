
// Copyright (c) 2023 Jiri Nytra (jiri.nytra at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <flux.hpp>

#include <cassert>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <variant>


struct comment_t
{
    std::string content;
};

struct section_t
{
    std::string name;
};

struct option_t
{
    std::string key;
    std::string value;
};

using token_t = std::variant<comment_t, section_t, option_t>;
using config_t = std::map<std::string, std::string, std::less<>>;

struct context_t
{
    std::string curr_section = "root";
    config_t config;
};

template <class... Ts> struct overloaded : Ts...
{
    using Ts::operator()...;
};

template <class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

bool not_blank_line(const std::string& line)
{
    auto space = [](auto c) { return std::isspace(c); };
    return not(line.empty() or flux::all(line, space));
}

token_t parse_line(const std::string& line)
{
    if (line.starts_with("#"))
        return comment_t{line.substr(1)};
    else if (line.starts_with("[") and line.ends_with("]"))
        return section_t{line.substr(1, line.size() - 2)};

    auto items = flux::ref(line).split_string("=").to<std::vector>();
    if (items.size() != 2)
        throw std::runtime_error("parse error");
    return option_t{std::string{items[0]}, std::string{items[1]}};
}

context_t add_to_config(context_t ctx, token_t tok)
{
    std::visit(overloaded{[&ctx](const section_t& s) { ctx.curr_section = s.name; },
                          [&ctx](const option_t& o) { ctx.config[ctx.curr_section + "." + o.key] = o.value; },
                          [](auto) { /* skip other tokens */}}, 
               tok);
    return ctx;
}

int main()
{
    std::stringstream ss;
    ss << "# This is example config file\n";
    ss << "project=example\n";
    ss << "language=C++\n";
    ss << "   \n";
    ss << "# server configuration \n";
    ss << "[server]\n";
    ss << "host=localhost\n";
    ss << "port=8080\n";
    ss << "\n";

    auto cfg = flux::getlines(ss)                      // read line by line
                    .filter(not_blank_line)            // skip all blank lines
                    .map(parse_line)                   // convert line to one of supported tokens
                    .fold(add_to_config, context_t{})  // fold tokens to the final variable map
                    .config;

    assert(cfg["root.project"] == "example");
    assert(cfg["root.language"] == "C++");
    assert(cfg["server.host"] == "localhost");
    assert(cfg["server.port"] == "8080");

    return 0;
}