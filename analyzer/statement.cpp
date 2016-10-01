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

#include "vapor/parser.h"
#include "vapor/analyzer/statement.h"
#include "vapor/analyzer/declaration.h"
#include "vapor/analyzer/expression.h"
#include "vapor/analyzer/import.h"
#include "vapor/analyzer/overload_set.h"
#include "vapor/analyzer/return.h"
#include "vapor/analyzer/symbol.h"
#include "vapor/analyzer/function.h"

std::unique_ptr<reaver::vapor::analyzer::_v1::statement> reaver::vapor::analyzer::_v1::preanalyze_statement(const reaver::vapor::parser::statement & parse, reaver::vapor::analyzer::_v1::scope * & lex_scope)
{
    return get<0>(fmap(parse.statement_value, make_overload_set(
        [&](const parser::declaration & decl) -> std::unique_ptr<statement>
        {
            auto ret = preanalyze_declaration(decl, lex_scope);
            return ret;
        },

        [&](const parser::return_expression & ret_expr) -> std::unique_ptr<statement>
        {
            auto ret = preanalyze_return(ret_expr, lex_scope);
            return ret;
        },

        [](const parser::expression_list & expr_list) -> std::unique_ptr<statement>
        {
            assert(0);
            return std::unique_ptr<expression>();
        },

        [&](const parser::function & func) -> std::unique_ptr<statement>
        {
            auto ret = preanalyze_function(func, lex_scope);
            return ret;
        },

        [](auto &&) -> std::unique_ptr<statement> { assert(0); }
    )));
}

