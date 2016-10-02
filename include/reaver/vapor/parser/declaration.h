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

#include <string>

#include "../range.h"
#include "expression_list.h"

namespace reaver
{
    namespace vapor
    {
        namespace parser { inline namespace _v1
        {
            struct declaration
            {
                range_type range;
                lexer::token identifier;
                optional<expression> type_expression;
                expression rhs;
            };

            inline bool operator==(const declaration & lhs, const declaration & rhs)
            {
                return lhs.range == rhs.range
                    && lhs.identifier == rhs.identifier
                    && lhs.type_expression == rhs.type_expression
                    && lhs.rhs == rhs.rhs;
            }

            declaration parse_declaration(context & ctx);

            void print(const declaration & decl, std::ostream & os, std::size_t indent = 0);
        }}
    }
}

