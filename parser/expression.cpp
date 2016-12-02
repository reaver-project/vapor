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

#include "vapor/parser/expression.h"
#include "vapor/parser/expr.h"

namespace reaver::vapor::parser
{
inline namespace _v1
{
    expression parse_expression(context & ctx, expression_special_modes mode)
    {
        expression ret;

        if (!peek(ctx))
        {
            throw expectation_failure{ "expression" };
        }

        auto type = peek(ctx)->type;

        switch (auto type = peek(ctx)->type)
        {
            case lexer::token_type::string:
                ret.expression_value = parse_literal<lexer::token_type::string>(ctx);
                break;

            case lexer::token_type::integer:
                ret.expression_value = parse_literal<lexer::token_type::integer>(ctx);
                break;

            case lexer::token_type::boolean:
                ret.expression_value = parse_literal<lexer::token_type::boolean>(ctx);
                break;

            case lexer::token_type::identifier:
                ret.expression_value = parse_postfix_expression(ctx, mode);
                break;

            case lexer::token_type::import:
                ret.expression_value = parse_import_expression(ctx);
                break;

            case lexer::token_type::lambda:
                ret.expression_value = parse_lambda_expression(ctx);
                break;

            case lexer::token_type::struct_:
                ret.expression_value = parse_struct_literal(ctx);
                break;

            default:
                if (is_unary_operator(type))
                {
                    ret.expression_value = parse_unary_expression(ctx);
                }

                else
                {
                    throw expectation_failure{ "expression", ctx.begin->string, ctx.begin->range };
                }
        }

        if (peek(ctx))
        {
            type = peek(ctx)->type;
            if (is_binary_operator(type) && !(mode == expression_special_modes::assignment && type == lexer::token_type::assign))
            {
                auto p1 = precedence({ type, operator_type::binary });
                auto p2 = ctx.operator_stack.size() ? make_optional(precedence(ctx.operator_stack.back())) : none;
                while (ctx.operator_stack.empty() || p1 < *p2 || (p1 == *p2 && associativity(type) == assoc::right))
                {
                    visit(
                        [&](const auto & value) -> unit {
                            ret.range = value.range;
                            return {};
                        },
                        ret.expression_value);
                    ret.expression_value = parse_binary_expression(ctx, std::move(ret));

                    if (!peek(ctx))
                    {
                        break;
                    }

                    type = peek(ctx)->type;
                    if (!is_binary_operator(type))
                    {
                        break;
                    }

                    p1 = precedence({ type, operator_type::binary });
                }
            }
        }

        visit(
            [&](const auto & value) -> unit {
                ret.range = value.range;
                return {};
            },
            ret.expression_value);

        return ret;
    }

    void print(const expression & expr, std::ostream & os, std::size_t indent)
    {
        auto in = std::string(indent, ' ');

        os << in << "`expression` at " << expr.range << '\n';
        os << in << "{\n";
        visit(
            [&](const auto & value) -> unit {
                print(value, os, indent + 4);
                return {};
            },
            expr.expression_value);
        os << in << "}\n";
    }
}
}
