/**
 * Vapor Compiler Licence
 *
 * Copyright © 2014 Michał "Griwes" Dominiak
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
#include <vector>

#include "vapor/range.h"
#include "vapor/parser/expression.h"
#include "vapor/parser/helpers.h"

namespace reaver
{
    namespace vapor
    {
        namespace parser { inline namespace _v1
        {
            struct expression_list
            {
                class range range;
                std::vector<expression> expressions;
            };

            template<typename Context>
            expression_list parse_expression_list(Context & ctx)
            {
                expression_list ret;

                ret.expressions.push_back(parse_expression(ctx));

                if (peek(ctx, lexer::token_type::comma))
                {
                    expect(ctx, lexer::token_type::comma);
                    ret.expressions.push_back(parse_expression(ctx));
                }

                ret.range = { ret.expressions.front().range.start(), ret.expressions.back().range.end() };

                return ret;
            }

            void print(const expression_list & list, std::ostream & os, std::size_t indent = 0)
            {
                auto in = std::string(indent, ' ');

                os << in << "`expression-list` at " << list.range << '\n';

                for (auto && expression : list.expressions)
                {
                    os << in << "{\n";
                    print(expression, os, indent + 4);
                    os << in << "}\n";
                }
            }
        }}
    }
}
