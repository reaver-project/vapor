/**
 * Vapor Compiler Licence
 *
 * Copyright © 2015-2017 Michał "Griwes" Dominiak
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

#include "vapor/parser/id_expression.h"

namespace reaver::vapor::parser
{
inline namespace _v1
{
    id_expression parse_id_expression(context & ctx)
    {
        id_expression ret;

        ret.id_expression_value.push_back(parse_literal<lexer::token_type::identifier>(ctx));

        while (peek(ctx, lexer::token_type::dot))
        {
            expect(ctx, lexer::token_type::dot);
            ret.id_expression_value.push_back(parse_literal<lexer::token_type::identifier>(ctx));
        }

        ret.range = { ret.id_expression_value.front().range.start(), ret.id_expression_value.back().range.end() };

        return ret;
    }

    void print(const id_expression & ide, std::ostream & os, print_context ctx)
    {
        os << styles::def << ctx << styles::rule_name << "id-expression";
        os << styles::def << " @ " << styles::address << &ide;
        os << styles::def << " (" << styles::range << ide.range << styles::def << "):\n";

        std::size_t idx = 0;
        for (auto && identifier : ide.id_expression_value)
        {
            print(identifier, os, ctx.make_branch(++idx == ide.id_expression_value.size()));
        }
    }
}
}
