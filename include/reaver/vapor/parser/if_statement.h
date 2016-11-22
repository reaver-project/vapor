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

#pragma once

#include "expression.h"

namespace reaver::vapor::parser { inline namespace _v1
{
    struct block;

    struct if_statement
    {
        range_type range;
        expression condition;
        recursive_wrapper<block> then_block;
        optional<recursive_wrapper<block>> else_block;
    };

    inline bool operator==(const if_statement & lhs, const if_statement & rhs)
    {
        return lhs.range == rhs.range
            && lhs.condition == rhs.condition
            && *lhs.then_block == rhs.then_block
            && lhs.else_block == rhs.else_block;
    }

    if_statement parse_if_statement(context & ctx);

    void print(const if_statement & stmt, std::ostream & os, std::size_t indent = 0);
}}

