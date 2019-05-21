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

#include "vapor/parser/lambda_expression.h"
#include "vapor/parser/block.h"
#include "vapor/parser/expr.h"

namespace reaver::vapor::parser
{
inline namespace _v1
{
    bool operator==(const lambda_expression & lhs, const lambda_expression & rhs)
    {
        return lhs.range == rhs.range && lhs.captures == rhs.captures && lhs.parameters == rhs.parameters
            && lhs.return_type == rhs.return_type && lhs.body == rhs.body;
    }

    lambda_expression parse_lambda_expression(context & ctx)
    {
        lambda_expression ret;

        auto start = expect(ctx, lexer::token_type::lambda).range.start();
        if (peek(ctx, lexer::token_type::square_bracket_open))
        {
            expect(ctx, lexer::token_type::square_bracket_close);
            if (!peek(ctx, lexer::token_type::square_bracket_close))
            {
                ret.captures = parse_capture_list(ctx);
            }
            expect(ctx, lexer::token_type::square_bracket_close);
        }

        if (peek(ctx, lexer::token_type::round_bracket_open))
        {
            expect(ctx, lexer::token_type::round_bracket_open);
            if (!peek(ctx, lexer::token_type::round_bracket_close))
            {
                ret.parameters = parse_parameter_list(ctx);
            }
            expect(ctx, lexer::token_type::round_bracket_close);
        }

        if (peek(ctx, lexer::token_type::indirection))
        {
            expect(ctx, lexer::token_type::indirection);
            ret.return_type = parse_expression(ctx, expression_special_modes::brace);
        }

        if (peek(ctx, lexer::token_type::block_value))
        {
            ret.body = parse_single_statement_block(ctx);
        }
        else
        {
            ret.body = parse_block(ctx);
        }

        ret.range = { start, ret.body.range.end() };

        return ret;
    }

    void print(const lambda_expression & expr, std::ostream & os, print_context ctx)
    {
        os << styles::def << ctx << styles::rule_name << "lambda-expression";
        print_address_range(os, expr);
        os << '\n';

        assert(!expr.captures);

        fmap(expr.parameters, [&](auto && parameters) {
            print(parameters, os, ctx.make_branch(false));
            return unit{};
        });
        print(expr.body, os, ctx.make_branch(true));
    }
}
}
