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

#include <boost/variant.hpp>

#include <reaver/visit.h>

#include "vapor/range.h"
#include "vapor/parser/id_expression.h"
#include "vapor/parser/expression_list.h"
#include "vapor/parser/helpers.h"
#include "vapor/parser/import_expression.h"
#include "vapor/parser/lambda_expression.h"
#include "vapor/parser/postfix_expression.h"
#include "vapor/parser/literal.h"

namespace reaver
{
    namespace vapor
    {
        namespace parser { inline namespace _v1
        {
            struct expression_list;

            struct expression
            {
                class range range;
                boost::variant<literal, postfix_expression, import_expression, lambda_expression> expression_value;
            };

            template<typename Context>
            expression parse_expression(Context & ctx)
            {
                expression ret;

                if (peek(ctx, lexer::token_type::string))
                {
                    ret.expression_value = parse_literal(ctx);
                }

                else if (peek(ctx, lexer::token_type::identifier))
                {
                    ret.expression_value = parse_postfix_expression(ctx);
                }

                else if (peek(ctx, lexer::token_type::import))
                {
                    ret.expression_value = parse_import_expression(ctx);
                }

                else if (peek(ctx, lexer::token_type::square_bracket_open))
                {
                    ret.expression_value = parse_lambda_expression(ctx);
                }

                else
                {
                    throw expectation_failure{ "expression", ctx.begin->string, ctx.begin->range };
                }

                visit([&](const auto & value) -> unit { ret.range = value.range; return {}; }, ret.expression_value);

                return ret;
            }

            void print(const expression & expr, std::ostream & os, std::size_t indent = 0)
            {
                auto in = std::string(indent, ' ');

                os << in << "`expression` at " << expr.range << '\n';
                os << in << "{\n";
                visit([&](const auto & value) -> unit { print(value, os, indent + 4); return {}; }, expr.expression_value);
                os << in << "}\n";
            }
        }}
    }
}
