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

#include "vapor/analyzer/expressions/call.h"
#include "vapor/analyzer/expressions/type.h"
#include "vapor/analyzer/symbol.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    std::unique_ptr<expression> call_expression::_clone_expr_with_replacement(replacements & repl) const
    {
        if (_replacement_expr)
        {
            return repl.claim(_replacement_expr.get());
        }

        auto ret = std::make_unique<owning_call_expression>(_function, fmap(_args, [&](auto arg) { return repl.copy_claim(arg); }));

        ret->_range = _range;

        if (_cloned_type_expr)
        {
            ret->_cloned_type_expr = repl.claim(_cloned_type_expr.get());
            auto type = ret->_cloned_type_expr->as<type_expression>();
            assert(type);
            ret->_set_type(type->get_value());
            return ret;
        }

        ret->_set_type(get_type());

        return ret;
    }

    future<expression *> call_expression::_simplify_expr(simplification_context & ctx)
    {
        if (_replacement_expr)
        {
            return make_ready_future(_replacement_expr.release());
        }

        return when_all(fmap(_args, [&](auto && arg) { return arg->simplify_expr(ctx); })).then([&](auto && repl) {
            assert(_args.size() == repl.size());
            for (std::size_t i = 0; i < _args.size(); ++i)
            {
                if (repl[i] && repl[i] != _args[i])
                {
                    _args[i] = repl[i];
                    ctx.something_happened();
                }
            }

            return _function->simplify(ctx, _args);
        });
    }

    future<expression *> owning_call_expression::_simplify_expr(simplification_context & ctx)
    {
        if (_replacement_expr)
        {
            return _replacement_expr->simplify_expr(ctx).then([&](auto && repl) -> expression * {
                replace_uptr(_replacement_expr, repl, ctx);
                return this;
            });
        }

        return when_all(fmap(_var_exprs, [&](auto && arg) { return arg->simplify_expr(ctx); })).then([&](auto && repl) {
            replace_uptrs(_var_exprs, repl, ctx);
            return this->call_expression::_simplify_expr(ctx);
        });
    }
}
}
