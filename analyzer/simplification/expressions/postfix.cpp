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

#include <reaver/prelude/fold.h>

#include "vapor/analyzer/expressions/expression_list.h"
#include "vapor/analyzer/expressions/identifier.h"
#include "vapor/analyzer/expressions/member.h"
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
        if (_call_expression)
        {
            return _call_expression->clone_expr_with_replacement(repl);
        }

        auto ret = std::unique_ptr<postfix_expression>(new postfix_expression(*this));

        ret->_base_expr = _base_expr->clone_expr_with_replacement(repl);
        ret->_arguments = fmap(_arguments, [&](auto && arg) { return arg->clone_expr_with_replacement(repl); });

        ret->_referenced_expression = fmap(_referenced_expression, [&](auto && var) {
            auto it = repl.expressions.find(var);
            if (it != repl.expressions.end())
            {
                return repl.expressions[var];
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
                    if (!_base_expr->is_constant())
                    {
                        return make_ready_future<expression *>(this);
                    }

                    assert(_referenced_expression.get()->is_member());
                    auto member = _base_expr->get_member(dynamic_cast<member_expression *>(*_referenced_expression)->get_name());
                    assert(member);

                    if (!member->is_constant())
                    {
                        return make_ready_future<expression *>(nullptr);
                    }

                    auto repl = replacements{};
                    return make_ready_future<expression *>(member->clone_expr_with_replacement(repl).release());
                }

                return _call_expression->simplify_expr(ctx).then([&](auto && repl) -> expression * {
                    replace_uptr(_call_expression, repl, ctx);
                    return this;
                });
            });
    }
}
}
