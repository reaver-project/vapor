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
            return repl.claim(_call_expression.get());
        }

        auto base = repl.claim(_base_expr.get());
        if (!_modifier)
        {
            return base;
        }

        assert(_arguments.empty());
        auto ret = std::unique_ptr<postfix_expression>(new postfix_expression(get_ast_info().get(), std::move(base), _modifier, {}, _accessed_member));

        auto type = ret->_base_expr->get_type();

        ret->_referenced_expression = fmap(_referenced_expression, [&](auto && expr) {
            if (auto replaced = repl.try_get_replacement(expr))
            {
                type = replaced->get_type();
                return replaced;
            }
            type = expr->get_type();
            return expr;
        });

        ret->_set_type(type);

        return ret;
    }

    future<expression *> postfix_expression::_simplify_expr(recursive_context ctx)
    {
        if (_call_expression)
        {
            replacements repl;
            auto clone = repl.claim(_call_expression.get()).release();

            return clone->simplify_expr(ctx).then([ctx, clone](auto && simplified) {
                if (simplified && simplified != clone)
                {
                    ctx.proper.keep_alive(clone);
                    return simplified;
                }
                return clone;
            });
        }

        return when_all(fmap(_arguments, [&, ctx](auto && expr) { return expr->simplify_expr(ctx); }))
            .then([&, ctx](auto && simplified) {
                replace_uptrs(_arguments, simplified, ctx.proper);
                return _base_expr->simplify_expr(ctx);
            })
            .then([&, ctx](auto && simplified) {
                replace_uptr(_base_expr, simplified, ctx.proper);

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
                    auto member = _base_expr->get_member(_referenced_expression.get()->as<member_expression>()->get_name());
                    assert(member);

                    if (!member->is_constant())
                    {
                        return make_ready_future<expression *>(nullptr);
                    }

                    auto repl = replacements{};
                    return make_ready_future<expression *>(repl.claim(member).release());
                }

                assert(0);
            });
    }
}
}
