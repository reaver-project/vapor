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

#include <boost/optional.hpp>

#include "vapor/range.h"
#include "vapor/parser/helpers.h"
#include "vapor/parser/argument_list.h"
#include "vapor/parser/capture_list.h"
#include "vapor/parser/block.h"

namespace reaver
{
    namespace vapor
    {
        namespace parser { inline namespace _v1
        {
            struct lambda_expression
            {
                class range range;
                boost::optional<capture_list> captures;
                boost::optional<argument_list> arguments;
                block body;
            };

            template<typename Context>
            lambda_expression parse_lambda_expression(Context & ctx)
            {
                lambda_expression ret;

                auto start = expect(ctx, lexer::token_type::square_bracket_open).range.start();
                if (!peek(ctx, lexer::token_type::square_bracket_close))
                {
                    ret.captures = parse_capture_list(ctx);
                }
                expect(ctx, lexer::token_type::square_bracket_close);

                expect(ctx, lexer::token_type::round_bracket_open);
                if (!peek(ctx, lexer::token_type::round_bracket_close))
                {
                    ret.arguments = parse_argument_list(ctx);
                }
                expect(ctx, lexer::token_type::round_bracket_close);

                if (peek(ctx, lexer::token_type::block_value))
                {
                    ret.body = parse_single_statement_block(ctx);
                }
                else
                {
                    ret.body = parse_block(ctx);
                }

                ret.range = { start, ret.body.range.end() };

                return ret;
            }

            void print(const lambda_expression & expr, std::ostream & os, std::size_t indent = 0)
            {
                auto in = std::string(indent, ' ');

                os << in << "`lambda-expression` at " << expr.range << '\n';

                assert(!expr.captures && !expr.arguments);

                os << in << "{\n";
                print(expr.body, os, indent + 4);
                os << in << "}\n";
            }
        }}
    }
}
