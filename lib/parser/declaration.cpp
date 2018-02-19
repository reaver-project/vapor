/**
 * Vapor Compiler Licence
 *
 * Copyright © 2015-2018 Michał "Griwes" Dominiak
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
    bool operator==(const declaration & lhs, const declaration & rhs)
    {
        return lhs.range == rhs.range && lhs.identifier == rhs.identifier && lhs.type_expression == rhs.type_expression && lhs.rhs == rhs.rhs;
    }

    declaration parse_declaration(context & ctx, declaration_mode mode)
    {
        declaration ret;

        if (peek(ctx, lexer::token_type::export_))
        {
            ret.export_ = expect(ctx, lexer::token_type::export_);
        }

        auto start = expect(ctx, lexer::token_type::let).range.start();
        ret.identifier = parse_literal<lexer::token_type::identifier>(ctx);

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
            if (mode != declaration_mode::member || !ret.type_expression)
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

    void print(const declaration & decl, std::ostream & os, print_context ctx)
    {
        os << styles::def << ctx << styles::rule_name << "declaration";
        print_address_range(os, decl);

        auto name_ctx = ctx.make_branch(!decl.type_expression && !decl.rhs);
        os << '\n' << name_ctx << styles::subrule_name << "name:\n";
        print(decl.identifier, os, name_ctx.make_branch(true));

        fmap(decl.type_expression, [&, ctx = ctx.make_branch(!decl.rhs)](auto && expr) {
            os << styles::def << ctx << styles::subrule_name << "type-specifier:\n";
            print(expr, os, ctx.make_branch(true));
            return unit{};
        });
        fmap(decl.rhs, [&, ctx = ctx.make_branch(true)](auto && rhs) {
            os << styles::def << ctx << styles::subrule_name << "initializer-expression:\n";
            print(rhs, os, ctx.make_branch(true));
            return unit{};
        });
    }
}
}
