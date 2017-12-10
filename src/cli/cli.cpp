/**
 * Vapor Compiler Licence
 *
 * Copyright © 2017 Michał "Griwes" Dominiak
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation is required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 **/

#include <boost/program_options.hpp>

#include "cli.h"

namespace reaver::vapor::cli
{
options_result get_options(int argc, char ** argv)
{
    // clang-format off
    boost::program_options::options_description general("General");
    general.add_options()
        ("help,h", "print this message")
        ("version,v", "print version information");

    boost::program_options::options_description io("Input and output");
    io.add_options()
        ("output,o", boost::program_options::value<std::string>(), "set the output file");

    boost::program_options::options_description hidden;
    hidden.add_options()
        ("input-file", boost::program_options::value<std::string>(), "");

    boost::program_options::positional_options_description positional;
    positional.add("input-file", 1);

    boost::program_options::options_description options;
    options.add(general).add(io).add(hidden);
    // clang-format on

    boost::program_options::variables_map variables;
    boost::program_options::store(
        boost::program_options::command_line_parser(argc, argv)
            .options(options)
            .positional(positional)
            .style(boost::program_options::command_line_style::allow_short | boost::program_options::command_line_style::allow_long
                | boost::program_options::command_line_style::allow_sticky | boost::program_options::command_line_style::allow_dash_for_short
                | boost::program_options::command_line_style::long_allow_next | boost::program_options::command_line_style::short_allow_next
                | boost::program_options::command_line_style::allow_long_disguise)
            .run(),
        variables);

    if (variables.count("help"))
    {
        std::cout << "Vapor compiler, version 0.0 (prerelease)\n";
        std::cout << general << '\n';
        std::cout << io << '\n';

        return { nullptr, true };
    }

    if (variables.count("version"))
    {
        std::cout << "Vapor compiler, version 0.0 (prerelease)\n";
        std::cout << "Distributed under modified zlib license.\n";

        return { nullptr, true };
    }

    auto ret = std::make_unique<config::compiler_options>(std::make_unique<config::language_options>());

    ret->set_source_path(variables["input-file"].as<std::string>());

    if (variables.count("output"))
    {
        ret->set_output_path(variables["output"].as<std::string>());
    }

    return { std::move(ret), false };
}
}
