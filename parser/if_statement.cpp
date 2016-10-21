/**
 * Vapor Compiler Licence
 *
 * Copyright © 2016 Michał "Griwes" Dominiak
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

#include "vapor/parser/if_statement.h"
#include "vapor/parser/lambda_expression.h"

reaver::vapor::parser::_v1::if_statement reaver::vapor::parser::_v1::parse_if_statement(reaver::vapor::parser::_v1::context & ctx)
{
    if_statement ret;

    auto start = expect(ctx, lexer::token_type::if_).range.start();

    expect(ctx, lexer::token_type::round_bracket_open);
    ret.condition = parse_expression(ctx);
    expect(ctx, lexer::token_type::round_bracket_close);

    ret.then_block = parse_block(ctx);

    if (peek(ctx, lexer::token_type::else_))
    {
        expect(ctx, lexer::token_type::else_);

        if (peek(ctx, lexer::token_type::if_))
        {
            auto alternative = parse_if_statement(ctx);
            block else_block;
            else_block.range = alternative.range;
            else_block.block_value.push_back(statement{ alternative.range, std::move(alternative) });
            ret.else_block = std::move(else_block);
        }

        else
        {
            ret.else_block = parse_block(ctx);
        }

        ret.range = { start, ret.else_block->range.end() };
    }

    else
    {
        ret.range = { start, ret.then_block->range.end() };
    }

    return ret;
}

void reaver::vapor::parser::_v1::print(const reaver::vapor::parser::_v1::if_statement & stmt, std::ostream & os, std::size_t indent)
{
    auto in = std::string(indent, ' ');

    os << in << "`if-statement`  at " << stmt.range << '\n';

    os << in << "`condition`:\n";
    os << in << "{\n";
    print(stmt.condition, os, indent + 4);
    os << in << "}\n";

    os << in << "`then` block:\n";
    os << in << "{\n";
    print(stmt.then_block, os, indent + 4);
    os << in << "}\n";

    if (stmt.else_block)
    {
        os << in << "`else` block:\n";
        os << in << "{\n";
        print(*stmt.else_block, os, indent + 4);
        os << in << "}\n";
    }
}

