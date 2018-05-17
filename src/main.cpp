/**
 * Vapor Compiler Licence
 *
 * Copyright © 2014-2018 Michał "Griwes" Dominiak
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

#include <fstream>

#include <boost/process.hpp>
#include <boost/program_options/errors.hpp>

#include "cli/cli.h"
#include "vapor/analyzer.h"
#include "vapor/codegen.h"
#include "vapor/config/compiler_options.h"
#include "vapor/lexer.h"
#include "vapor/parser.h"
#include "vapor/utf.h"

void run_process(const std::string & cmdline)
{
    reaver::logger::dlog() << "Running `" << cmdline << "`";

    auto child = boost::process::child(cmdline, boost::process::std_out > stdout, boost::process::std_err > stderr);

    child.wait();

    if (child.exit_code() != 0)
    {
        throw reaver::exception{ reaver::logger::error } << "`" << cmdline << "` failed";
    }
}

int main(int argc, char ** argv) try
{
    // force a single thread of execution
    reaver::default_executor(reaver::make_executor<reaver::thread_pool>(1));

    auto[options, exit] = reaver::vapor::cli::get_options(argc, argv);

    if (exit)
    {
        return 0;
    }

    // compiler_options should probably expose an ifstream, or maybe just the entire
    // program buffer loaded into memory
    // but I don't know which one is better right now
    // would be useful for compiling from stdin
    assert(options->source_path());

    std::ifstream input(options->source_path()->string());
    if (!input)
    {
        reaver::logger::dlog(reaver::logger::error) << "couldn't open the source file";
        return 1;
    }

    std::string program_utf8{ std::istreambuf_iterator<char>(input.rdbuf()), std::istreambuf_iterator<char>() };
    auto program = boost::locale::conv::utf_to_utf<char32_t>(program_utf8);

    reaver::logger::dlog() << "Input:";
    reaver::logger::dlog() << program_utf8;
    reaver::logger::dlog();

    reaver::logger::dlog() << "Tokens:";
    reaver::vapor::lexer::iterator iterator{ program.begin(), program.end(), options->source_path()->native() };
    for (auto it = iterator; it; ++it)
    {
        reaver::logger::dlog() << *it;
    }
    reaver::logger::dlog();

    reaver::logger::default_logger().sync();

    reaver::logger::dlog() << "AST:";
    auto ast = reaver::vapor::parser::parse_ast(iterator);
    reaver::logger::dlog() << std::ref(ast);

    reaver::logger::default_logger().sync();

    reaver::logger::dlog() << "Analyzed AST:";
    reaver::vapor::analyzer::ast analyzed_ast{ std::move(ast), *options };
    analyzed_ast.analyze();
    reaver::logger::dlog() << std::ref(analyzed_ast);

    reaver::logger::default_logger().sync();

    reaver::logger::dlog() << "Simplified AAST:";
    analyzed_ast.simplify();
    reaver::logger::dlog() << std::ref(analyzed_ast);

    reaver::logger::default_logger().sync();

    // only create the module interface file if there is an actual input file
    if (options->source_path())
    {
        reaver::logger::dlog() << "Generating module interface file...";
        auto module_path = options->module_path();
        boost::filesystem::create_directories(module_path.parent_path());
        std::ofstream interface_file{ module_path.string() };
        if (!interface_file)
        {
            reaver::logger::dlog(reaver::logger::error) << "couldn't open module interface output file";
            return 1;
        }
        analyzed_ast.serialize_to(interface_file);
        reaver::logger::dlog() << "Done.";
    }

    auto ir = analyzed_ast.codegen_ir();

    reaver::vapor::codegen::result generated_ir{ ir, reaver::vapor::codegen::make_printer() };
    reaver::logger::dlog() << "Generated IR:";
    reaver::logger::dlog() << generated_ir;

    reaver::vapor::codegen::result generated_code{ ir, reaver::vapor::codegen::make_llvm_ir() };
    reaver::logger::dlog() << "Generated LLVM IR:";
    reaver::logger::dlog() << generated_code;

    namespace modes = reaver::vapor::config::compilation_modes;

    auto name = options->llvm_path();

    {
        boost::filesystem::create_directories(name.parent_path());
        std::ofstream out{ name.string(), std::ios::trunc | std::ios::out };
        out << generated_code;
    }

    // figure out the path for llc and opt
    boost::process::ipstream stream;
    auto child = boost::process::child("llvm-config --bindir", boost::process::std_out > stream);
    child.wait();

    std::string llvm_bin_dir;

    if (child.exit_code() != 0 || !std::getline(stream, llvm_bin_dir))
    {
        throw reaver::exception{ reaver::logger::fatal } << "llvm-config failed";
    }

    boost::filesystem::path lbd = llvm_bin_dir;

    if (options->compilation_mode() == modes::assembly || options->should_generate_assembly_file())
    {
        auto name = options->assembly_path();

        boost::filesystem::create_directories(name.parent_path());
        run_process((lbd / "llc").string() + " " + options->llvm_path().string() + " -filetype=asm -relocation-model=pic -o " + name.string());
    }

    if (options->compilation_mode() >= modes::object)
    {
        auto name = options->compilation_mode() == modes::object ? options->binary_path() : options->object_path();

        boost::filesystem::create_directories(name.parent_path());
        run_process((lbd / "llc").string() + " " + options->llvm_path().string() + " -filetype=obj -relocation-model=pic -o " + name.string());
    }

    if (!options->should_generate_llvm_ir_file())
    {
        boost::filesystem::remove(options->llvm_path());
    }

    if (options->compilation_mode() >= modes::link)
    {
        boost::filesystem::create_directories(options->binary_path().parent_path());
        assert("need to figure this shit out, yo");
    }

    reaver::logger::default_logger().sync();
}

catch (reaver::exception & e)
{
    e.print(reaver::logger::default_logger());
    reaver::logger::default_logger().sync();

    if (e.level() >= reaver::logger::crash)
    {
        return 2;
    }

    return 1;
}

catch (boost::program_options::error & e)
{
    reaver::logger::dlog(reaver::logger::error) << e.what();
    reaver::logger::default_logger().sync();

    return 1;
}

catch (std::exception & e)
{
    reaver::logger::dlog(reaver::logger::crash) << e.what();
    reaver::logger::default_logger().sync();

    return 2;
}
