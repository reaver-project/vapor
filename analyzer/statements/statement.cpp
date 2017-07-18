/**
 * Vapor Compiler Licence
 *
 * Copyright © 2016-2017 Michał "Griwes" Dominiak
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

#include "vapor/analyzer/statements/statement.h"
#include "vapor/analyzer/expressions/expression.h"
#include "vapor/analyzer/expressions/import.h"
#include "vapor/analyzer/function.h"
#include "vapor/analyzer/statements/declaration.h"
#include "vapor/analyzer/statements/function_declaration.h"
#include "vapor/analyzer/statements/if.h"
#include "vapor/analyzer/statements/return.h"
#include "vapor/analyzer/symbol.h"
#include "vapor/parser.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    std::unique_ptr<statement> preanalyze_statement(const parser::statement & parse, scope *& lex_scope)
    {
        return get<0>(fmap(parse.statement_value,
            make_overload_set(
                [&](const parser::declaration & decl) -> std::unique_ptr<statement> {
                    auto ret = preanalyze_declaration(decl, lex_scope);
                    return ret;
                },

                [&](const parser::return_expression & ret_expr) -> std::unique_ptr<statement> {
                    auto ret = preanalyze_return(ret_expr, lex_scope);
                    return ret;
                },

                [](const parser::expression_list & expr_list) -> std::unique_ptr<statement> {
                    assert(0);
                    return std::unique_ptr<expression>();
                },

                [&](const parser::function & func) -> std::unique_ptr<statement> {
                    auto ret = preanalyze_function(func, lex_scope);
                    return ret;
                },

                [&](const parser::if_statement & if_stmt) -> std::unique_ptr<statement> {
                    auto ret = preanalyze_if_statement(if_stmt, lex_scope);
                    return ret;
                },

                [](auto &&) -> std::unique_ptr<statement> { assert(0); })));
    }

    std::unique_ptr<statement> statement::clone_with_replacement(const std::vector<expression *> & params, const std::vector<expression *> & args) const
    {
        replacements repl;

        assert(params.size() == args.size());
        for (std::size_t i = 0; i < params.size(); ++i)
        {
            repl.statements[params[i]] = args[i];
            if (auto param_expr = dynamic_cast<expression *>(params[i]))
            {
                auto arg_expr = dynamic_cast<expression *>(args[i]);
                assert(arg_expr);
                repl.expressions[param_expr] = arg_expr;
            }
        }

        return clone_with_replacement(repl);
    }
}
}
