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

#include <boost/process.hpp>
#include <boost/program_options.hpp>

#include "cli.h"

namespace reaver::vapor::cli
{
options_result get_options(int argc, char ** argv)
{
    auto ret = std::make_unique<config::compiler_options>(std::make_unique<config::language_options>());

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
        ("m", boost::program_options::value<std::string>()->value_name("module-interface")
            ->notifier([&](auto val){ ret->set_module_path(std::move(val)); }),
            "set the module interface output file")
        ("l", boost::program_options::value<std::string>()->value_name("[ llvm-ir-output ]")->implicit_value("")
            ->notifier([&](auto val){ ret->set_llvm_path(std::move(val)); }),
            "set the LLVM output file; if the argument is nonexistant or an empty string, forces generation of an LLVM IR file at the default path")
        ("a", boost::program_options::value<std::string>()->value_name("[ assembly-output ]")->implicit_value("")
            ->notifier([&](auto val){ ret->set_assembly_path(std::move(val)); }),
            "set the assembly output file; if the argument is nonexistant or an empty string, forces generation of an assembly file at the default path; "
            "only valid if mode is at least -s")
        ("o", boost::program_options::value<std::string>()->value_name("binary-output")
            ->notifier([&](auto val){ ret->set_binary_path(std::move(val)); }),
            "set the object or executable output file")

        ("mdir", boost::program_options::value<std::string>()->value_name("module-output-dir")
            ->notifier([&](auto val){ ret->set_module_dir(std::move(val)); }),
            "set the module output directory (controls the default module interface output file's location, overriden by -m)")
        ("ldir", boost::program_options::value<std::string>()->value_name("llvm-ir-output-dir")
            ->notifier([&](auto val){ ret->set_llvm_dir(std::move(val)); }),
            "set the LLVM output directory (controls the default LLVM output file's location, overriden by -l)")
        ("adir", boost::program_options::value<std::string>()->value_name("assembly-output-dir")
            ->notifier([&](auto val){ ret->set_assembly_dir(std::move(val)); }),
            "set the assembly output directory (control the default assembly output file's location, oberriden by -s)")
        ("odir", boost::program_options::value<std::string>()->value_name("binary-output-dir")
            ->notifier([&](auto val){ ret->set_binary_dir(std::move(val)); }),
            "set the object file output directory (controls the default output file's location, overriden by -o)")

        ("outdir", boost::program_options::value<std::string>()->value_name("output-directories"),
            "set all the output directories to this value (overriden by more specific -{m,l,o}dir flags)")

        ("module-path,I", boost::program_options::value<std::vector<std::string>>()->value_name("search-path")->composing()
            ->notifier([&](auto val){ for (auto && elem : val) { ret->add_module_path(std::move(elem)); } }),
            "provided additional module search paths")
        ("llvm-config", boost::program_options::value<std::string>()->value_name("llvm-config")
            ->notifier([&](auto val){ ret->set_output_dir(std::move(val)); }),
            "specify a custom `llvm-config` path, used to find `llc`")
    ;

    boost::program_options::options_description hidden;
    hidden.add_options()
        ("input-file", boost::program_options::value<std::string>()
            ->notifier([&](auto val){ ret->set_source_path(std::move(val)); }), "")
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
    boost::program_options::notify(variables);

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

    if (!variables.count("input-file"))
    {
        throw exception{ logger::error } << "no source files provided!";
    }

    auto compilation_mode = (variables.count("i") << 0) | (variables.count("s") << 1) | (variables.count("c") << 2);
    switch (compilation_mode)
    {
        case config::compilation_modes::llvm_ir:
        case config::compilation_modes::assembly:
        case config::compilation_modes::object:
            ret->set_compilation_mode(static_cast<config::modes_enum>(compilation_mode));
            break;

        case 0:
            throw exception{ logger::error } << "can't link with vprc yet";
            break;

        default:
            throw exception{ logger::error } << "multiple compilation modes selected; choose at most one of -i, -s and -o.";
    }

    ret->set_compilation_handler([& ctx = *ret, vprc = argv[0]](const boost::filesystem::path & path) {
        std::vector<std::string> argv;

        argv.push_back(vprc);
        argv.push_back("-c");
        argv.push_back(path.string());

#define HANDLE_DIR(name, flag)                                                                                                                                 \
    auto name##_variable_from_macro = ctx.name##_dir();                                                                                                        \
    if (name##_variable_from_macro)                                                                                                                            \
    {                                                                                                                                                          \
        argv.push_back(flag);                                                                                                                                  \
        argv.push_back(name##_variable_from_macro.value().string());                                                                                           \
    }

        HANDLE_DIR(module, "--mdir");
        HANDLE_DIR(llvm, "--ldir");
        HANDLE_DIR(assembly, "--adir");
        HANDLE_DIR(binary, "--odir");

#undef HANDLE_DIR

        logger::dlog() << "Compiling dependency: " << path << "...";
        logger::default_logger().sync();

        namespace bp = boost::process;
        auto child = bp::child(argv, bp::std_out > stdout, bp::std_err > stderr, bp::std_in < stdin);

        child.wait();
        if (child.exit_code() != 0)
        {
            assert(!"TODO: nice error message when a dependency fails to compile");
        }
    });

    return { std::move(ret), false };
}
}
