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

#include <memory>

#include "vapor/parser/expression_list.h"
#include "vapor/parser/expression.h"
#include "vapor/analyzer/scope.h"
#include "vapor/analyzer/helpers.h"
#include "vapor/analyzer/literal.h"
#include "vapor/analyzer/import.h"
#include "vapor/analyzer/lambda.h"

namespace reaver
{
    namespace vapor
    {
        namespace analyzer { inline namespace _v1
        {
            class postfix_expression
            {
            };

            struct expression_recursive_wrapper;

            using expression = shptr_variant<
                std::vector<expression_recursive_wrapper>, // expression-list
                literal,
                import_expression,
                postfix_expression,
                lambda
            >;

            struct expression_recursive_wrapper
            {
                expression value;
            };

            expression preanalyze_expression(const parser::expression & expr, const std::shared_ptr<scope> & lex_scope)
            {
                return visit(make_visitor(
                    id<parser::string_literal>(), [](auto && string) -> expression
                    {
                        assert(0);
                        return std::shared_ptr<literal>();
                    },

                    id<parser::integer_literal>(), [](auto && integer) -> expression
                    {
                        assert(0);
                        return std::shared_ptr<literal>();
                    },

                    id<parser::postfix_expression>(), [](auto && postfix) -> expression
                    {
                        assert(0);
                        return std::shared_ptr<postfix_expression>();
                    },

                    id<parser::import_expression>(), [](auto && import) -> expression
                    {
                        assert(0);
                        return std::shared_ptr<import_expression>();
                    },

                    id<parser::lambda_expression>(), [](auto && lambda_expr) -> expression
                    {
                        assert(0);
                        return std::shared_ptr<lambda>();
                    }
                ), expr.expression_value);
            }

            expression preanalyze_expression(const parser::expression_list & expr, const std::shared_ptr<scope> & lex_scope)
            {
                if (expr.expressions.size() > 1)
                {
                    return preanalyze_expression(expr, lex_scope);
                }

                auto ret = std::make_shared<std::vector<expression_recursive_wrapper>>();
                ret->reserve(expr.expressions.size());
                std::transform(expr.expressions.begin(), expr.expressions.end(), std::back_inserter(*ret), [&](auto && expr)
                {
                    return expression_recursive_wrapper{ preanalyze_expression(expr, lex_scope) };
                });
                return ret;
            }
        }}
    }
}
