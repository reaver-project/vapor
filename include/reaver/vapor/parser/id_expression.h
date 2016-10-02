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

#include <vector>
#include <string>

#include "../range.h"
#include "../lexer/token.h"
#include "helpers.h"

namespace reaver
{
    namespace vapor
    {
        namespace parser { inline namespace _v1
        {
            struct id_expression
            {
                range_type range;
                std::vector<lexer::token> id_expression_value;
            };

            inline bool operator==(const id_expression & lhs, const id_expression & rhs)
            {
                return lhs.range == rhs.range && lhs.id_expression_value == rhs.id_expression_value;
            }

            id_expression parse_id_expression(context & ctx);

            void print(const id_expression & ide, std::ostream & os, std::size_t indent = 0);
        }}
    }
}

