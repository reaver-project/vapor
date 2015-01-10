/**
 * Vapor Compiler Licence
 *
 * Copyright © 2015 Michał "Griwes" Dominiak
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

reaver::vapor::parser::_v1::lambda_expression reaver::vapor::parser::_v1::parse_lambda_expression(reaver::vapor::parser::_v1::context & ctx)
{
    lambda_expression ret;

    auto start = expect(ctx, lexer::token_type::square_bracket_open).range.start();
    if (!peek(ctx, lexer::token_type::square_bracket_close))
    {
        ret.captures = parse_capture_list(ctx);
    }
    expect(ctx, lexer::token_type::square_bracket_close);

    if (peek(ctx, lexer::token_type::round_bracket_open))
    {
        expect(ctx, lexer::token_type::round_bracket_open);
        if (!peek(ctx, lexer::token_type::round_bracket_close))
        {
            ret.arguments = parse_argument_list(ctx);
        }
        expect(ctx, lexer::token_type::round_bracket_close);
    }

    if (peek(ctx, lexer::token_type::block_value))
    {
        ret.body = parse_single_statement_block(ctx);
    }
    else
    {
        ret.body = parse_block(ctx);
    }

    ret.range = { start, ret.body.get().range.end() };

    return ret;
}

void reaver::vapor::parser::_v1::print(const reaver::vapor::parser::_v1::lambda_expression & expr, std::ostream & os, std::size_t indent)
{
    auto in = std::string(indent, ' ');

    os << in << "`lambda-expression` at " << expr.range << '\n';

    assert(!expr.captures && !expr.arguments);

    os << in << "{\n";
    print(expr.body.get(), os, indent + 4);
    os << in << "}\n";
}
