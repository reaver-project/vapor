/**
 * Vapor Compiler Licence
 *
 * Copyright © 2015-2017 Michał "Griwes" Dominiak
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

#include "vapor/parser/parameter_list.h"
#include "vapor/parser/expr.h"

namespace reaver::vapor::parser
{
inline namespace _v1
{
    parameter_list parse_parameter_list(context & ctx)
    {
        parameter_list ret;

        while (!peek(ctx, lexer::token_type::round_bracket_close))
        {
            auto name = expect(ctx, lexer::token_type::identifier);
            expect(ctx, lexer::token_type::colon);
            auto type_expr = parse_expression(ctx);

            auto range = range_type{ name.range.start(), type_expr.range.end() };
            ret.parameters.push_back(parameter{ std::move(range), std::move(name), std::move(type_expr) });

            if (peek(ctx, lexer::token_type::comma))
            {
                expect(ctx, lexer::token_type::comma);
            }
        }

        assert(!ret.parameters.empty());
        ret.range = range_type{ ret.parameters.front().range.start(), ret.parameters.back().range.end() };

        return ret;
    }

    void print(const parameter_list & arglist, std::ostream & os, std::size_t indent)
    {
        auto in = std::string(indent, ' ');

        os << in << "`parameter-list` at " << arglist.range << '\n';
        os << in << "{\n";
        fmap(arglist.parameters, [&, in = std::string(indent + 4, ' ')](auto && parameter) {
            os << in << "{\n";
            print(parameter.name, os, indent + 8);
            print(parameter.type, os, indent + 8);
            os << in << "}\n";
            return unit{};
        });
        os << in << "}\n";
    }
}
}
