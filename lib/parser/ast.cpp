/**
 * Vapor Compiler Licence
 *
 * Copyright © 2018-2019 Michał "Griwes" Dominiak
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

#include "vapor/parser/ast.h"
#include "vapor/parser/import_expression.h"
#include "vapor/parser/module.h"
#include "vapor/print_helpers.h"

// these are only included to work around https://gcc.gnu.org/bugzilla/show_bug.cgi?id=80654
// TODO: stop this overzealous madness once GCC 7 is dropped
#include "vapor/parser/lambda_expression.h"
#include "vapor/parser/struct.h"

namespace reaver::vapor::parser
{
inline namespace _v1
{
    ast parse_ast(lexer::iterator begin, lexer::iterator end)
    {
        ast ret;

        auto ctx = context{ begin, end, {} };

        while (peek(ctx, lexer::token_type::import))
        {
            ret.global_imports.push_back(parse_import_expression(ctx));
            expect(ctx, lexer::token_type::semicolon);
        }

        while (ctx.begin != ctx.end)
        {
            ret.module_definitions.push_back(parse_module(ctx));
        }

        return ret;
    }

    std::ostream & operator<<(std::ostream & os, const ast & ast)
    {
        auto ctx = print_context{};

        os << styles::subrule_name << "import statements:\n";
        std::size_t idx = 0;
        for (auto && import : ast.global_imports)
        {
            print(import, os, ctx.make_branch(++idx == ast.global_imports.size()));
        }

        os << styles::subrule_name << "module definitions:\n";
        idx = 0;
        for (auto && module : ast.module_definitions)
        {
            print(module, os, ctx.make_branch(++idx == ast.module_definitions.size()));
        }

        return os;
    }
}
}
