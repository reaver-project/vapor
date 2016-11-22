/**
 * Vapor Compiler Licence
 *
 * Copyright © 2015-2016 Michał "Griwes" Dominiak
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

#include "vapor/parser/argument_list.h"
#include "vapor/parser/lambda_expression.h"

namespace reaver::vapor::parser { inline namespace _v1
{
    argument_list parse_argument_list(context & ctx)
    {
        argument_list ret;

        while (!peek(ctx, lexer::token_type::round_bracket_close))
        {
            auto name = expect(ctx, lexer::token_type::identifier);
            expect(ctx, lexer::token_type::colon);
            auto type_expr = parse_expression(ctx);

            auto range = range_type{ name.range.start(), type_expr.range.end() };
            ret.arguments.push_back(argument{ std::move(range), std::move(name), std::move(type_expr) });

            if (peek(ctx, lexer::token_type::comma))
            {
                expect(ctx, lexer::token_type::comma);
            }
        }

        assert(!ret.arguments.empty());
        ret.range = range_type{ ret.arguments.front().range.start(), ret.arguments.back().range.end() };

        return ret;
    }

    void print(const argument_list & arglist, std::ostream & os, std::size_t indent)
    {
        auto in = std::string(indent, ' ');

        os << in << "`argument-list` at " << arglist.range << '\n';
        os << in << "{\n";
        fmap(arglist.arguments, [&, in = std::string(indent + 4, ' ')](auto && argument) {
            os << in << "{\n";
            print(argument.name, os, indent + 8);
            print(argument.type, os, indent + 8);
            os << in << "}\n";
            return unit{};
        });
        os << in << "}\n";
    }
}}

