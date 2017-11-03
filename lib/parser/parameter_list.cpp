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

#include "vapor/parser/parameter_list.h"
#include "vapor/parser/expr.h"

namespace reaver::vapor::parser
{
inline namespace _v1
{
    parameter_list parse_parameter_list(context & ctx)
    {
        parameter_list ret;

        while (!peek(ctx, lexer::token_type::round_bracket_close))
        {
            auto name = parse_literal<lexer::token_type::identifier>(ctx);
            expect(ctx, lexer::token_type::colon);
            auto type_expr = parse_expression(ctx);

            auto range = range_type{ name.range.start(), type_expr.range.end() };
            ret.parameters.push_back(parameter{ std::move(range), std::move(name), std::move(type_expr) });

            if (peek(ctx, lexer::token_type::comma))
            {
                expect(ctx, lexer::token_type::comma);
            }
        }

        assert(!ret.parameters.empty());
        ret.range = range_type{ ret.parameters.front().range.start(), ret.parameters.back().range.end() };

        return ret;
    }

    void print(const parameter_list & arglist, std::ostream & os, print_context ctx)
    {
        os << styles::def << ctx << styles::rule_name << "parameter-list";
        print_address_range(os, arglist);
        os << '\n';

        std::size_t idx = 0;
        for (auto && parameter : arglist.parameters)
        {
            auto param_ctx = ctx.make_branch(++idx == arglist.parameters.size());
            os << styles::def << param_ctx << styles::subrule_name << "parameter:\n";

            print(parameter.name, os, param_ctx.make_branch(false));
            print(parameter.type, os, param_ctx.make_branch(true));
        }
    }
}
}
