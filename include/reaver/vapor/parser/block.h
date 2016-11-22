/**
 * Vapor Compiler Licence
 *
 * Copyright © 2014-2016 Michał "Griwes" Dominiak
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

#include "../range.h"
#include "helpers.h"
#include "statement.h"
#include "expression_list.h"

namespace reaver::vapor::parser { inline namespace _v1
{
    struct statement;

    struct block
    {
        range_type range;
        std::vector<variant<recursive_wrapper<block>, recursive_wrapper<statement>>> block_value;
        optional<expression_list> value_expression;
    };

    inline bool operator==(const block & lhs, const block & rhs)
    {
        return lhs.range == rhs.range
            && lhs.block_value == rhs.block_value
            && lhs.value_expression == rhs.value_expression;
    }

    block parse_block(context & ctx);
    block parse_single_statement_block(context & ctx);

    void print(const expression_list & list, std::ostream & os, std::size_t indent);
    void print(const block & bl, std::ostream & os, std::size_t indent);
}}

