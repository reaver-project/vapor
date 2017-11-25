/**
 * Vapor Compiler Licence
 *
 * Copyright © 2014-2017 Michał "Griwes" Dominiak
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
#include "expression_list.h"
#include "helpers.h"
#include "statement.h"

namespace reaver::vapor::parser
{
inline namespace _v1
{
    struct statement;

    struct block
    {
        range_type range;
        std::vector<std::variant<recursive_wrapper<block>, recursive_wrapper<statement>>> block_value;
        std::optional<expression_list> value_expression;
    };

    bool operator==(const block & lhs, const block & rhs);

    block parse_block(context & ctx);
    block parse_single_statement_block(context & ctx);

    void print(const expression_list & list, std::ostream & os, print_context ctx);
    void print(const block & bl, std::ostream & os, print_context ctx);
}
}
