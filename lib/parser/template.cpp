/**
 * Vapor Compiler Licence
 *
 * Copyright © 2017 Michał "Griwes" Dominiak
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

#include "vapor/parser/template.h"

namespace reaver::vapor::parser
{
inline namespace _v1
{
    bool operator==(const template_introducer & lhs, const template_introducer & rhs)
    {
        return lhs.range == rhs.range && lhs.template_parameters == rhs.template_parameters;
    }

    bool operator==(const template_expression & lhs, const template_expression & rhs)
    {
        return lhs.range == rhs.range && lhs.parameters == rhs.parameters && lhs.expression == rhs.expression;
    }

    template_introducer parse_template_introducer(context & ctx)
    {
        template_introducer ret;

        auto start = expect(ctx, lexer::token_type::with).range.start();
        expect(ctx, lexer::token_type::round_bracket_open);
        ret.template_parameters = parse_parameter_list(ctx);
        auto end = expect(ctx, lexer::token_type::round_bracket_close).range.end();

        ret.range = { start, end };

        return ret;
    }

    namespace
    {
        struct named_template_expression
        {
            optional<identifier> name;
            template_expression value;
        };

        enum class template_type
        {
            named,
            unnamed
        };

        named_template_expression parse_template(context & ctx, template_type tpl_type)
        {
            named_template_expression ret;

            ret.value.parameters = parse_template_introducer(ctx);

            if (!peek(ctx))
            {
                throw expectation_failure{ tpl_type == template_type::named ? "declaration" : "expression" };
            }

            switch (auto type = peek(ctx)->type)
            {
                case lexer::token_type::typeclass:
                case lexer::token_type::implicit:
                {
                    if (tpl_type == template_type::named)
                    {
                        auto def = parse_typeclass_definition(ctx);
                        ret.name = std::move(def.name);
                        ret.value.expression = std::move(def.definition);
                    }

                    else
                    {
                        ret.value.expression = parse_typeclass_literal(ctx);
                    }

                    break;
                }

                default:
                    throw expectation_failure{ tpl_type == template_type::named ? "declaration" : "expression", ctx.begin->string, ctx.begin->range };
            }

            return ret;
        }
    }

    template_expression parse_template_expression(context & ctx)
    {
        return parse_template(ctx, template_type::unnamed).value;
    }

    declaration parse_template_declaration(context & ctx)
    {
        auto template_ = parse_template(ctx, template_type::named);

        declaration decl;

        decl.identifier = std::move(template_.name.get());
        decl.range = template_.value.range;
        decl.rhs = expression{ template_.value.range, std::move(template_.value) };

        return decl;
    }

    void print(const template_introducer & expr, std::ostream & os, print_context ctx)
    {
        os << styles::def << ctx << styles::rule_name << "template-introducer";
        print_address_range(os, expr);
        os << '\n';

        auto params_ctx = ctx.make_branch(true);
        os << styles::def << params_ctx << styles::subrule_name << "parameters:\n";
        print(expr.template_parameters, os, params_ctx.make_branch(true));
    }

    void print(const template_expression & expr, std::ostream & os, print_context ctx)
    {
        os << styles::def << ctx << styles::rule_name << "template-expression";
        print_address_range(os, expr);
        os << '\n';

        auto intro_ctx = ctx.make_branch(false);
        os << styles::def << intro_ctx << styles::subrule_name << "introducer:\n";
        print(expr.parameters, os, intro_ctx.make_branch(true));

        auto expr_ctx = ctx.make_branch(true);
        os << styles::def << expr_ctx << styles::subrule_name << "expression:\n";
        fmap(expr.expression, [&](auto && expr) {
            print(expr, os, expr_ctx.make_branch(true));
            return unit{};
        });
    }
}
}
