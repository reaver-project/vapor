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

#include <numeric>

#include <reaver/prelude/fold.h>

#include "vapor/analyzer/expressions/expression_list.h"
#include "vapor/analyzer/statements/block.h"
#include "vapor/analyzer/statements/return.h"
#include "vapor/analyzer/symbol.h"
#include "vapor/parser.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    void block::_ensure_cache() const
    {
        if (_is_clone_cache)
        {
            return;
        }

        std::lock_guard<std::mutex> lock{ _clone_cache_lock };
        if (_clone)
        {
            return;
        }

        auto clone = std::unique_ptr<block>(new block(*this));
        auto repl = replacements{};

        clone->_statements = fmap(_statements, [&](auto && stmt) { return stmt->clone_with_replacement(repl); });
        clone->_value_expr = fmap(_value_expr, [&](auto && expr) { return expr->clone_expr_with_replacement(repl); });
        clone->_is_clone_cache = true;

        _clone = std::move(clone);
    }

    std::unique_ptr<statement> block::_clone_with_replacement(replacements & repl) const
    {
        _ensure_cache();

        if (!_is_clone_cache)
        {
            return _clone.get()->clone_with_replacement(repl);
        }

        auto ret = std::unique_ptr<block>(new block(*this));

        ret->_statements = fmap(_statements, [&](auto && stmt) { return stmt->clone_with_replacement(repl); });
        ret->_value_expr = fmap(_value_expr, [&](auto && expr) { return expr->clone_expr_with_replacement(repl); });

        return ret;
    }

    future<statement *> block::_simplify(simplification_context & ctx)
    {
        _ensure_cache();

        auto fut = foldl(_statements, make_ready_future(true), [&](auto future, auto && statement) {
            return future.then([&](bool do_continue) {
                if (!do_continue)
                {
                    return make_ready_future(false);
                }

                return statement->simplify(ctx).then([&](auto && simplified) {
                    replace_uptr(statement, simplified, ctx);
                    return !statement->always_returns();
                });
            });
        });

        fmap(_value_expr, [&](auto && expr) {
            fut = fut.then([&, expr = expr.get()](bool do_continue) {
                if (!do_continue)
                {
                    return make_ready_future(false);
                }

                return expr->simplify_expr(ctx).then([&](auto && simplified) {
                    replace_uptr(*_value_expr, simplified, ctx);
                    return true;
                });
            });
            return unit{};
        });

        return fut.then([&](bool reached_end) -> statement * {
            if (!reached_end)
            {
                auto always_returning = std::find_if(_statements.begin(), _statements.end(), [](auto && stmt) { return stmt->always_returns(); });

                assert(always_returning != _statements.end());
                _statements.erase(always_returning + 1, _statements.end());
            }

            return this;
        });
    }
}
}
