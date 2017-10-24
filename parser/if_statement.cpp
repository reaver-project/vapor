/**
 * Vapor Compiler Licence
 *
 * Copyright © 2016-2017 Michał "Griwes" Dominiak
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
#include "vapor/parser/expr.h"

namespace reaver::vapor::parser
{
inline namespace _v1
{
    bool operator==(const if_statement & lhs, const if_statement & rhs)
    {
        return lhs.range == rhs.range && lhs.condition == rhs.condition && *lhs.then_block == rhs.then_block && lhs.else_block == rhs.else_block;
    }

    if_statement parse_if_statement(context & ctx)
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

    void print(const if_statement & stmt, std::ostream & os, print_context ctx)
    {
        os << styles::def << ctx << styles::rule_name << "if-statement";
        print_address_range(os, stmt);
        os << '\n';

        auto condition_ctx = ctx.make_branch(false);
        os << styles::def << condition_ctx << styles::subrule_name << "condition:\n";
        print(stmt.condition, os, condition_ctx.make_branch(true));

        auto then_ctx = ctx.make_branch(!stmt.else_block);
        os << styles::def << then_ctx << styles::subrule_name << "then-block:\n";
        print(stmt.then_block, os, then_ctx.make_branch(true));

        if (stmt.else_block)
        {
            auto else_ctx = ctx.make_branch(true);
            os << styles::def << else_ctx << styles::subrule_name << "else-block:\n";
            print(*stmt.else_block, os, ctx.make_branch(true));
        }
    }
}
}
