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

namespace reaver::vapor::parser
{
inline namespace _v1
{
    struct unary_expression
    {
        unary_expression() = default;

        // without this, GCC 6.4 complains that unary_expression has a deleted copy constructor
        // W. T. F.
        unary_expression(const unary_expression & expr) = default;
        unary_expression(unary_expression &&) = default;
        unary_expression & operator=(const unary_expression &) = default;
        unary_expression & operator=(unary_expression &&) = default;

        range_type range;
        lexer::token op;
        expression operand;
    };

    inline bool operator==(const unary_expression & lhs, const unary_expression & rhs)
    {
        return lhs.range == rhs.range && lhs.op == rhs.op && lhs.operand == rhs.operand;
    }

    const std::vector<lexer::token_type> & unary_operators();

    inline bool is_unary_operator(lexer::token_type t)
    {
        return std::find(unary_operators().begin(), unary_operators().end(), t) != unary_operators().end();
    }

    unary_expression parse_unary_expression(context & ctx);

    void print(const unary_expression & expr, std::ostream & os, print_context ctx);
}
}
