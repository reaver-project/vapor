/**
 * Vapor Compiler Licence
 *
 * Copyright © 2016 Michał "Griwes" Dominiak
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

#include "vapor/analyzer/expression.h"
#include "vapor/analyzer/closure.h"
#include "vapor/analyzer/binary_expression.h"
#include "vapor/analyzer/integer.h"
#include "vapor/analyzer/postfix_expression.h"
#include "vapor/analyzer/unary_expression.h"
#include "vapor/analyzer/import.h"

std::shared_ptr<reaver::vapor::analyzer::_v1::expression> reaver::vapor::analyzer::_v1::preanalyze_expression(const reaver::vapor::parser::_v1::expression & expr, const std::shared_ptr<reaver::vapor::analyzer::_v1::scope> & lex_scope)
{
    return get<0>(fmap(expr.expression_value, make_overload_set(
        [](const parser::string_literal & string) -> std::shared_ptr<expression>
        {
            assert(0);
            return nullptr;
        },

        [](const parser::integer_literal & integer) -> std::shared_ptr<expression>
        {
            return std::make_shared<integer_literal>(integer);
        },

        [](const parser::postfix_expression & postfix) -> std::shared_ptr<expression>
        {
            assert(0);
            return std::shared_ptr<postfix_expression>();
        },

        [](const parser::import_expression & import) -> std::shared_ptr<expression>
        {
            assert(0);
            return std::shared_ptr<import_expression>();
        },

        [&](const parser::lambda_expression & lambda_expr) -> std::shared_ptr<expression>
        {
            auto lambda = preanalyze_closure(lambda_expr, lex_scope);
            return lambda;
        },

        [](const parser::unary_expression & unary_expr) -> std::shared_ptr<expression>
        {
            assert(0);
            return std::shared_ptr<unary_expression>();
        },

        [&](const parser::binary_expression & binary_expr) -> std::shared_ptr<expression>
        {
            auto binexpr = preanalyze_binary_expression(binary_expr, lex_scope);
            return binexpr;
        }
    )));
}

