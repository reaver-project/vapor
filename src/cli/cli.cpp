/**
 * Vapor Compiler Licence
 *
 * Copyright © 2017-2018 Michał "Griwes" Dominiak
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
        ("version,v", "print version information")
    ;

    boost::program_options::options_description mode("Compilation mode");
    mode.add_options()
        ("i", "compile down to LLVM IR")
        ("s", "compile down to assembly")
        ("c", "compile down to an object file")
    ;

    boost::program_options::options_description io("Input and output");
    io.add_options()
        ("m", boost::program_options::value<std::string>(), "set the module interface output file")
        ("l", boost::program_options::value<std::string>(), "set the LLVM output file")
        ("a", boost::program_options::value<std::string>(), "set the assembly output file")
        ("o", boost::program_options::value<std::string>(), "set the output file")

        ("mdir", boost::program_options::value<std::string>(), "set the module output directory (controls the default module interface output file's location, overriden by -m)")
        ("ldir", boost::program_options::value<std::string>(), "set the LLVM output directory (controls the default LLVM output file's location, overriden by -l)")
        ("adir", boost::program_options::value<std::string>(), "set the assembly output directory (control the default assembly output file's location, oberriden by -s)")
        ("odir", boost::program_options::value<std::string>(), "set the object file output directory (controls the default output file's location, overriden by -o)")

        ("outdir", boost::program_options::value<std::string>(), "set all the output directories to this value (overriden by more specific -{m,l,o}dir flags)")

        ("module-path,I", boost::program_options::value<std::vector<std::string>>()->composing(), "provided additional module search paths")
        ("llvm-config", boost::program_options::value<std::string>(), "specify a custom `llvm-config` path, used to find `llc`")
    ;

    boost::program_options::options_description hidden;
    hidden.add_options()
        ("input-file", boost::program_options::value<std::string>(), "")
    ;

    boost::program_options::positional_options_description positional;
    positional.add("input-file", 1);

    boost::program_options::options_description options;
    options.add(general).add(mode).add(io).add(hidden);
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
        std::cout << mode << '\n';
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

    if (!variables.count("input-file"))
    {
        throw exception{ logger::error } << "no source files provided!";
    }

    ret->set_source_path(variables["input-file"].as<std::string>());

    auto compilation_mode = (variables.count("i") << 0) | (variables.count("s") << 1) | (variables.count("c") << 2);
    switch (compilation_mode)
    {
        case config::compilation_modes::llvm_ir:
        case config::compilation_modes::assembly:
        case config::compilation_modes::object:
            ret->set_compilation_mode(static_cast<config::modes_enum>(compilation_mode));
            break;

        case config::compilation_modes::link:
            throw exception{ logger::error } << "directly linking with vprc is not supported yet; use -i, -s or -o.";

        default:
            throw exception{ logger::error } << "multiple compilation modes selected; choose at most one of -i, -s and -o.";
    }

#define HANDLE_NAME(name, option)                                                                                                                              \
    if (variables.count(option))                                                                                                                               \
    {                                                                                                                                                          \
        ret->set_##name##_path(variables[option].as<std::string>());                                                                                           \
    }

    HANDLE_NAME(module, "m");
    HANDLE_NAME(llvm, "l");
    HANDLE_NAME(assembly, "a");
    HANDLE_NAME(object, "o");

#undef HANDLE_NAME

#define HANDLE_DIR(name, option)                                                                                                                               \
    if (variables.count(option))                                                                                                                               \
    {                                                                                                                                                          \
        ret->set_##name##_dir(variables[option].as<std::string>());                                                                                            \
    }

    HANDLE_DIR(module, "mdir");
    HANDLE_DIR(llvm, "ldir");
    HANDLE_DIR(assembly, "adir");
    HANDLE_DIR(object, "odir");

#undef HANDLE_NAME

    if (variables.count("outdir"))
    {
        ret->set_output_dir(variables["outdir"].as<std::string>());
    }

    if (variables.count("module-path"))
    {
        for (auto && path : variables["module-path"].as<std::vector<std::string>>())
        {
            ret->add_module_path(std::move(path));
        }
    }

    return { std::move(ret), false };
}
}
