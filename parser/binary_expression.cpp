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

#include "vapor/parser/binary_expression.h"
#include "vapor/parser/expr.h"

namespace reaver::vapor::parser
{
inline namespace _v1
{
    const std::vector<reaver::vapor::lexer::_v1::token_type> & binary_operators()
    {
        static std::vector<lexer::token_type> binary_ops = { lexer::token_type::plus,
            lexer::token_type::minus,
            lexer::token_type::star,
            lexer::token_type::slash,
            lexer::token_type::modulo,
            lexer::token_type::bitwise_and,
            lexer::token_type::bitwise_or,
            lexer::token_type::bitwise_xor,
            lexer::token_type::logical_and,
            lexer::token_type::logical_or,
            lexer::token_type::left_shift,
            lexer::token_type::right_shift,

            lexer::token_type::assign,
            lexer::token_type::plus_assignment,
            lexer::token_type::minus_assignment,
            lexer::token_type::star_assignment,
            lexer::token_type::slash_assignment,
            lexer::token_type::modulo_assignment,
            lexer::token_type::bitwise_and_assignment,
            lexer::token_type::bitwise_or_assignment,
            lexer::token_type::bitwise_xor_assignment,
            lexer::token_type::logical_and_assignment,
            lexer::token_type::logical_or_assignment,
            lexer::token_type::left_shift_assignment,
            lexer::token_type::right_shift_assignment,

            lexer::token_type::equals,
            lexer::token_type::not_equals,
            lexer::token_type::less,
            lexer::token_type::less_equal,
            lexer::token_type::greater,
            lexer::token_type::greater_equal,

            lexer::token_type::indirection,
            lexer::token_type::map,
            lexer::token_type::bind };

        return binary_ops;
    }

    const std::unordered_map<reaver::vapor::lexer::_v1::token_type, std::size_t> & binary_operator_precedences()
    {
        static std::unordered_map<lexer::token_type, std::size_t> precedences = []() {
            std::unordered_map<lexer::token_type, std::size_t> precedences;

            precedences[lexer::token_type::indirection] = 0;

            precedences[lexer::token_type::star] = 10;
            precedences[lexer::token_type::slash] = 10;
            precedences[lexer::token_type::modulo] = 10;

            precedences[lexer::token_type::plus] = 15;
            precedences[lexer::token_type::minus] = 15;

            precedences[lexer::token_type::left_shift] = 20;
            precedences[lexer::token_type::right_shift] = 20;

            precedences[lexer::token_type::less] = 25;
            precedences[lexer::token_type::less_equal] = 25;
            precedences[lexer::token_type::greater] = 25;
            precedences[lexer::token_type::greater_equal] = 25;

            precedences[lexer::token_type::equals] = 30;
            precedences[lexer::token_type::not_equals] = 30;

            precedences[lexer::token_type::bitwise_and] = 35;
            precedences[lexer::token_type::bitwise_xor] = 40;
            precedences[lexer::token_type::bitwise_or] = 45;
            precedences[lexer::token_type::logical_and] = 50;
            precedences[lexer::token_type::logical_or] = 55;

            for (auto t : { lexer::token_type::assign,
                     lexer::token_type::plus_assignment,
                     lexer::token_type::minus_assignment,
                     lexer::token_type::star_assignment,
                     lexer::token_type::slash_assignment,
                     lexer::token_type::left_shift_assignment,
                     lexer::token_type::right_shift_assignment,
                     lexer::token_type::bitwise_and_assignment,
                     lexer::token_type::bitwise_not_assignment,
                     lexer::token_type::bitwise_or_assignment,
                     lexer::token_type::bitwise_xor_assignment,
                     lexer::token_type::logical_and_assignment,
                     lexer::token_type::logical_or_assignment })
            {
                precedences[t] = 60;
            }

            return precedences;
        }();

        return precedences;
    }

    binary_expression parse_binary_expression(context & ctx, expression lhs)
    {
        binary_expression ret;

        ret.lhs = std::move(lhs);
        ret.op = expect(ctx, peek(ctx)->type);
        ctx.operator_stack.push_back({ ret.op.type, operator_type::binary });
        ret.rhs = parse_expression(ctx);
        ctx.operator_stack.pop_back();
        ret.range = { ret.lhs.range.start(), ret.rhs.range.end() };

        return ret;
    }

    void print(const binary_expression & expr, std::ostream & os, std::size_t indent)
    {
        auto in = std::string(indent, ' ');

        os << in << "`binary-expression` at " << expr.range << '\n';
        os << in << "{\n";
        print(expr.lhs, os, indent + 4);
        print(expr.op, os, indent + 4);
        print(expr.rhs, os, indent + 4);
        os << in << "}\n";
    }
}
}
