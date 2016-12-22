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

#include <reaver/prelude/fold.h>

#include "vapor/analyzer/expressions/expression_list.h"
#include "vapor/analyzer/expressions/identifier.h"
#include "vapor/analyzer/expressions/postfix.h"
#include "vapor/analyzer/function.h"
#include "vapor/analyzer/helpers.h"
#include "vapor/parser.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    std::unique_ptr<expression> postfix_expression::_clone_expr_with_replacement(replacements & repl) const
    {
        auto ret = std::unique_ptr<postfix_expression>(new postfix_expression(*this));

        ret->_base_expr = _base_expr->clone_expr_with_replacement(repl);
        ret->_arguments = fmap(_arguments, [&](auto && arg) { return arg->clone_expr_with_replacement(repl); });

        ret->_referenced_variable = fmap(_referenced_variable, [&](auto && var) {
            auto it = repl.variables.find(var);
            if (it != repl.variables.end())
            {
                return repl.variables[var];
            }
            return var;
        });

        return ret;
    }

    future<expression *> postfix_expression::_simplify_expr(simplification_context & ctx)
    {
        return when_all(fmap(_arguments, [&](auto && expr) { return expr->simplify_expr(ctx); }))
            .then([&](auto && simplified) {
                replace_uptrs(_arguments, simplified, ctx);
                return _base_expr->simplify_expr(ctx);
            })
            .then([&](auto && simplified) {
                replace_uptr(_base_expr, simplified, ctx);

                if (!_modifier)
                {
                    return make_ready_future(_base_expr.release());
                }

                if (_accessed_member)
                {
                    return make_ready_future<expression *>(this);
                }

                auto args = fmap(_arguments, [&](auto && expr) { return expr->get_variable(); });
                return _overload->simplify(ctx, std::move(args)).then([&](auto && simplified) -> expression * {
                    if (simplified)
                    {
                        return simplified;
                    }

                    return this;
                });
            });
    }
}
}
