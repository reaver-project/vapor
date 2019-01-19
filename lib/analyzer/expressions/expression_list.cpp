/**
 * Vapor Compiler Licence
 *
 * Copyright © 2016-2019 Michał "Griwes" Dominiak
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

#include <reaver/prelude/fold.h>

#include "vapor/analyzer/expressions/expression_list.h"
#include "vapor/analyzer/helpers.h"
#include "vapor/analyzer/semantic/symbol.h"
#include "vapor/parser/expr.h"
#include "vapor/parser/expression_list.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    void expression_list::print(std::ostream & os, print_context ctx) const
    {
        os << styles::def << ctx << styles::rule_name << "expression-list";
        print_address_range(os, this);
        os << '\n';

        auto type_ctx = ctx.make_branch(true);
        os << styles::def << type_ctx << styles::subrule_name << "type:\n";
        get_type()->print(os, type_ctx.make_branch(true));

        auto exprs_ctx = ctx.make_branch(true);
        os << styles::def << exprs_ctx << styles::subrule_name << "value:\n";

        std::size_t idx = 0;
        for (auto && expr : value)
        {
            expr->print(os, exprs_ctx.make_branch(++idx == value.size()));
        }
    }

    std::unique_ptr<expression> preanalyze_expression_list(precontext & ctx, const parser::expression_list & expr, scope * lex_scope)
    {
        if (expr.expressions.size() == 1)
        {
            return preanalyze_expression(ctx, expr.expressions.front(), lex_scope);
        }

        auto ret = std::make_unique<expression_list>(make_node(expr));
        ret->value = fmap(expr.expressions, [&](auto && expr) { return preanalyze_expression(ctx, expr, lex_scope); });
        ret->_set_ast_info(make_node(expr));
        return ret;
    }
}
}
