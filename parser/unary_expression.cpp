/**
 * Vapor Compiler Licence
 *
 * Copyright © 2015-2016 Michał "Griwes" Dominiak
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

#include "vapor/parser/unary_expression.h"
#include "vapor/parser/expr.h"

namespace reaver::vapor::parser
{
inline namespace _v1
{
    const std::vector<reaver::vapor::lexer::_v1::token_type> & unary_operators()
    {
        static std::vector<lexer::token_type> unary_ops = { lexer::token_type::plus,
            lexer::token_type::minus,
            lexer::token_type::logical_not,
            lexer::token_type::bitwise_not,
            lexer::token_type::star,
            lexer::token_type::increment,
            lexer::token_type::decrement };

        return unary_ops;
    }

    unary_expression parse_unary_expression(context & ctx)
    {
        unary_expression ret;

        auto t = peek(ctx)->type;
        if (is_unary_operator(t))
        {
            ret.op = expect(ctx, t);
            ctx.operator_stack.push_back({ ret.op.type, operator_type::unary });
            ret.operand = parse_expression(ctx);
            ctx.operator_stack.pop_back();
        }

        else
        {
            throw expectation_failure{ "unary-expression", ctx.begin->string, ctx.begin->range };
        }

        ret.range = { ret.op.range.start(), ret.operand.range.end() };

        return ret;
    }

    void print(const unary_expression & expr, std::ostream & os, std::size_t indent)
    {
        auto in = std::string(indent, ' ');

        os << in << "`unary-expression` at " << expr.range << '\n';
        os << in << "{\n";
        print(expr.op, os, indent + 4);
        print(expr.operand, os, indent + 4);
        os << in << "}\n";
    }
}
}
