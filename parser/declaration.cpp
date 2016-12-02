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

#include "vapor/parser/declaration.h"
#include "vapor/parser/expr.h"

namespace reaver::vapor::parser
{
inline namespace _v1
{
    declaration parse_declaration(context & ctx, declaration_mode mode)
    {
        declaration ret;

        auto start = expect(ctx, lexer::token_type::let).range.start();
        ret.identifier = expect(ctx, lexer::token_type::identifier);

        if (peek(ctx, lexer::token_type::colon))
        {
            expect(ctx, lexer::token_type::colon);
            ret.type_expression = parse_expression(ctx, expression_special_modes::assignment);
        }

        if (peek(ctx, lexer::token_type::assign))
        {
            expect(ctx, lexer::token_type::assign);
            ret.rhs = parse_expression(ctx);
            ret.range = { start, ret.rhs->range.end() };
        }

        else
        {
            if (mode != declaration_mode::member_declaration || !ret.type_expression)
            {
                const char * message = "a type specifier or an initializer expression";

                if (!peek(ctx))
                {
                    throw expectation_failure{ message };
                }

                throw expectation_failure{ message, ctx.begin->string, ctx.begin->range };
            }

            ret.range = { start, ret.type_expression->range.end() };
        }

        return ret;
    }

    void print(const declaration & decl, std::ostream & os, std::size_t indent)
    {
        auto in = std::string(indent, ' ');

        os << in << "`declaration` at " << decl.range << '\n';

        os << in << "{\n";
        print(decl.identifier, os, indent + 4);
        fmap(decl.type_expression, [&](auto && expr) {
            auto in = std::string(indent + 4, ' ');
            os << in << "`type-specifier`:\n";
            os << in << "{\n";
            print(expr, os, indent + 8);
            os << in << "}\n";
            return unit{};
        });
        fmap(decl.rhs, [&](auto && rhs) {
            auto in = std::string(indent + 4, ' ');
            os << in << "`initializer-expression`:\n";
            os << in << "{\n";
            print(rhs, os, indent + 8);
            os << in << "}\n";
            return unit{};
        });
        os << in << "}\n";
    }
}
}
