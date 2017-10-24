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

#pragma once

#include "../lexer/token.h"
#include "../range.h"
#include "expression.h"
#include "unary_expression.h"

namespace reaver::vapor::parser
{
inline namespace _v1
{
    struct binary_expression
    {
        range_type range;
        lexer::token op;
        expression lhs;
        expression rhs;
    };

    bool operator==(const binary_expression & lhs, const binary_expression & rhs);

    const std::vector<lexer::token_type> & binary_operators();

    inline bool is_binary_operator(lexer::token_type t)
    {
        return std::find(binary_operators().begin(), binary_operators().end(), t) != binary_operators().end();
    }

    const std::unordered_map<lexer::token_type, std::size_t> & binary_operator_precedences();

    inline std::size_t precedence(operator_context ctx)
    {
        if (ctx.type == operator_type::unary)
        {
            return 5;
        }

        return binary_operator_precedences().at(ctx.op);
    }

    enum class assoc
    {
        right,
        left
    };

    inline assoc associativity(lexer::token_type type)
    {
        static std::vector<lexer::token_type> right_assoc = { lexer::token_type::assign,
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
            lexer::token_type::logical_or_assignment };

        return std::find(right_assoc.begin(), right_assoc.end(), type) != right_assoc.end() ? assoc::right : assoc::left;
    };

    binary_expression parse_binary_expression(context & ctx, expression lhs);

    void print(const binary_expression & expr, std::ostream & os, print_context ctx);
}
}
