/**
 * Vapor Compiler Licence
 *
 * Copyright © 2014-2016 Michał "Griwes" Dominiak
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

#include "vapor/utf8.h"
#include "vapor/lexer.h"
#include "vapor/parser.h"
#include "vapor/analyzer.h"
#include "vapor/codegen.h"

std::u32string program = UR"program(module hello_world
{
    function fibonacci(n : int)
    {
        if (n == 0)
        {
            return 0;
        }

        if (n == 1)
        {
            return 1;
        }

        if (n == 2)
        {
            return 1;
        }

        return fibonacci(n - 1) + fibonacci(n - 2);
    }

    let entry = λ(arg : int)
    {
        let constant_foldable = fibonacci(7);
        let non_constant_foldable = fibonacci(arg);

        return constant_foldable + non_constant_foldable;
    };
})program";

int main() try
{
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

    auto ir = analyzed_ast.codegen_ir();
    reaver::vapor::codegen::result generated_code{ ir, reaver::vapor::codegen::make_cxx() };

    reaver::logger::dlog() << "Codegen IR:";
    reaver::logger::dlog() << ir;

    reaver::logger::dlog() << "Generated code:";
    reaver::logger::dlog() << generated_code;

    std::ofstream out{ "output/output.cpp", std::ios::trunc | std::ios::out };
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

