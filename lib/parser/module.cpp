/**
 * Vapor Compiler Licence
 *
 * Copyright © 2016-2018 Michał "Griwes" Dominiak
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

#include "vapor/parser/module.h"

namespace reaver::vapor::parser
{
inline namespace _v1
{
    module parse_module(context & ctx)
    {
        module ret;

        auto start = expect(ctx, lexer::token_type::module).range.start();
        ret.name = parse_id_expression(ctx);

        expect(ctx, lexer::token_type::curly_bracket_open);

        while (!peek(ctx, lexer::token_type::curly_bracket_close))
        {
            ret.statements.push_back(parse_statement(ctx, statement_mode::module));
        }

        auto end = expect(ctx, lexer::token_type::curly_bracket_close).range.end();

        ret.range = { start, end };

        return ret;
    }

    void print(const module & mod, std::ostream & os, print_context ctx)
    {
        os << styles::def << ctx << styles::rule_name << "module";
        print_address_range(os, mod);

        auto name_ctx = ctx.make_branch(mod.statements.empty());
        os << '\n' << name_ctx << styles::subrule_name << "name:\n";
        print(mod.name, os, name_ctx.make_branch(true));

        if (mod.statements.size())
        {
            auto stmts_ctx = ctx.make_branch(true);
            os << styles::def << stmts_ctx << styles::subrule_name << "statements:\n";

            std::size_t idx = 0;
            for (auto && statement : mod.statements)
            {
                print(statement, os, stmts_ctx.make_branch(++idx == mod.statements.size()));
            }
        }
    }
}
}
