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

#include <vector>
#include <string>

#include "vapor/range.h"
#include "vapor/lexer/token.h"
#include "vapor/parser/helpers.h"

namespace reaver
{
    namespace vapor
    {
        namespace parser { inline namespace _v1
        {
            struct id_expression
            {
                class range range;
                std::vector<lexer::token> id_expression_value;
            };

            template<typename Context>
            id_expression parse_id_expression(Context & ctx)
            {
                id_expression ret;

                ret.id_expression_value.push_back(expect(ctx, lexer::token_type::identifier));

                while (peek(ctx, lexer::token_type::dot))
                {
                    expect(ctx, lexer::token_type::dot);
                    ret.id_expression_value.push_back(std::move(expect(ctx, lexer::token_type::identifier)));
                }

                ret.range = { ret.id_expression_value.front().range.start(), ret.id_expression_value.back().range.end() };

                return ret;
            };

            void print(const id_expression & ide, std::ostream & os, std::size_t indent = 0)
            {
                auto in = std::string(indent, ' ');

                os << in << "`id-expression` at " << ide.range << '\n';
                os << in << "{\n";
                for (auto && identifier : ide.id_expression_value)
                {
                    os << std::string(indent + 4, ' ') << identifier << '\n';
                }
                os << in << "}\n";
            }
        }}
    }
}
