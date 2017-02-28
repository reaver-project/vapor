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
#include "expression.h"
#include "helpers.h"

namespace reaver::vapor::parser
{
inline namespace _v1
{
    struct parameter
    {
        range_type range;
        lexer::token name;
        expression type;
    };

    struct parameter_list
    {
        range_type range;
        std::vector<parameter> parameters;
    };

    inline bool operator==(const parameter & lhs, const parameter & rhs)
    {
        return lhs.range == rhs.range && lhs.name == rhs.name && lhs.type == rhs.type;
    }

    inline bool operator==(const parameter_list & lhs, const parameter_list & rhs)
    {
        return lhs.range == rhs.range && lhs.parameters == rhs.parameters;
    }

    parameter_list parse_parameter_list(context & ctx);

    void print(const parameter_list &, std::ostream &, std::size_t indent);
}
}
