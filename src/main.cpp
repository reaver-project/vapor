/**
 * Vapor Compiler Licence
 *
 * Copyright © 2014-2017 Michał "Griwes" Dominiak
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

#include "vapor/analyzer.h"
#include "vapor/codegen.h"
#include "vapor/config/compiler_options.h"
#include "vapor/lexer.h"
#include "vapor/parser.h"
#include "vapor/utf.h"

std::u32string program = UR"program(module hello_world
{
    let int32 = sized_int(32);

    let mn = struct { let m : int32; let n : int32; };

    function ackermann(args : mn) -> int32
    {
        if (args.m == 0)
        {
            return args.n + 1;
        }

        if (args.n == 0)
        {
            return ackermann(args{ .m = .m - 1, .n = 1 });
        }

        return ackermann(args{ .m = .m - 1, .n = ackermann(args{ .n = .n - 1 }) });
    }

    let entry = λ(arg : int32) -> int32
    {
        let constant_foldable = ackermann(mn{ 2, 3 });
        let non_constant_foldable = ackermann(mn{ arg, arg + 1 });

        return constant_foldable - non_constant_foldable;
    };
})program";

int main() try
{
    // force a single thread of execution
    reaver::default_executor(reaver::make_executor<reaver::thread_pool>(1));

    reaver::logger::dlog() << "Input:";
    reaver::logger::dlog() << reaver::vapor::utf8(program);
    reaver::logger::dlog();

    reaver::logger::dlog() << "Tokens:";
    reaver::vapor::lexer::iterator iterator{ program.begin(), program.end() };
    for (auto it = iterator; it; ++it)
    {
        reaver::logger::dlog() << *it;
    }
    reaver::logger::dlog();

    reaver::logger::default_logger().sync();

    reaver::logger::dlog() << "AST:";
    reaver::vapor::parser::ast ast{ iterator };
    reaver::logger::dlog() << ast;

    reaver::logger::default_logger().sync();

    reaver::logger::dlog() << "Analyzed AST:";
    reaver::vapor::analyzer::ast analyzed_ast{ std::move(ast) };
    reaver::logger::dlog() << std::ref(analyzed_ast);

    reaver::logger::default_logger().sync();

    reaver::logger::dlog() << "Simplified AAST:";
    analyzed_ast.simplify();
    reaver::logger::dlog() << std::ref(analyzed_ast);

    reaver::logger::default_logger().sync();

    auto ir = analyzed_ast.codegen_ir();

    reaver::vapor::codegen::result generated_ir{ ir, reaver::vapor::codegen::make_printer() };
    reaver::logger::dlog() << "Generated IR:";
    reaver::logger::dlog() << generated_ir;

    reaver::vapor::codegen::result generated_code{ ir, reaver::vapor::codegen::make_llvm_ir() };
    reaver::logger::dlog() << "Generated LLVM IR:";
    reaver::logger::dlog() << generated_code;

    boost::filesystem::create_directories("output");
    std::ofstream out{ "output/output.ll", std::ios::trunc | std::ios::out };
    out << generated_code;

    reaver::logger::default_logger().sync();
}

catch (reaver::exception & e)
{
    e.print(reaver::logger::default_logger());

    if (e.level() >= reaver::logger::crash)
    {
        return 2;
    }

    return 1;
}

catch (std::exception & e)
{
    reaver::logger::dlog(reaver::logger::crash) << e.what();

    return 2;
}
