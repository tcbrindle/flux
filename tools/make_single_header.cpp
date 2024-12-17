// make_single_header.cpp
//
// Copyright (c) 2018 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <deque>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <regex>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace {

constexpr auto& include_regex = R"(#include <(flux(?:/\w*)+.h(?:pp)?)>)";

template <typename Rng, typename Value>
bool contains(const Rng& range, const Value& val)
{
    return std::find(range.begin(), range.end(), val) != std::end(range);
}

struct include_processor {

    static std::string run(const fs::path& start_file)
    {
        auto start_path = start_file;
        start_path.remove_filename();
        return include_processor{std::move(start_path)}.process_one(start_file);
    }

private:
    struct replacement {
        std::ptrdiff_t pos;
        std::ptrdiff_t len;
        std::string text;
    };

    include_processor(fs::path&& start_path)
        : start_path_(std::move(start_path))
    {}

    std::string process_one(const fs::path& path)
    {
        std::cout << "Processing path " << path << '\n';

        std::ifstream infile(path);

        std::string text(std::istreambuf_iterator<char>{infile},
                         std::istreambuf_iterator<char>{});

        std::deque<replacement> replacements;

        std::for_each(std::sregex_iterator(text.begin(), text.end(), regex_),
                       std::sregex_iterator{},
                       [&] (const auto& match) {
            auto rep = replacement{match.position(), match.length()};

            auto new_path = start_path_;
            new_path = fs::canonical(new_path.append(match.str(1)));

            if (!contains(processed_paths_, new_path)) {
                rep.text = process_one(new_path);
            }

            replacements.push_back(std::move(rep));
        });

        process_replacements(text, replacements);
        processed_paths_.push_back(path);
        return text;
    }

    static void process_replacements(std::string& str, std::deque<replacement>& replacements)
    {
        std::sort(replacements.begin(), replacements.end(), [](const auto& lhs, const auto& rhs) {
            return lhs.pos < rhs.pos;
        });

        while (!replacements.empty()) {
            auto rep = std::move(replacements.front());
            replacements.pop_front();
            str.replace(rep.pos, rep.len, rep.text);

            for (auto& r : replacements) {
                r.pos -= rep.len - rep.text.length();
            }
        }
    }

    fs::path start_path_;
    std::regex regex_{include_regex};
    std::vector<fs::path> processed_paths_;
};

}

int main(int argc, char** argv) try
{
    if (argc != 3) {
        std::cerr << "Usage: make_single_header IN_FILE.hpp OUT_FILE.hpp\n";
        return 1;
    }

    const auto infile_path = fs::canonical(fs::path(argv[1]));
    const auto outfile_path = fs::path(argv[2]);

    const auto out_str =  include_processor::run(infile_path);

    std::ofstream outfile(outfile_path);
    std::copy(out_str.begin(), out_str.end(), std::ostreambuf_iterator<char>(outfile));

} catch (const std::exception& ex) {
    std::cout << ex.what() << '\n';
}