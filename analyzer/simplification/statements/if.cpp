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

#include "vapor/analyzer/statements/if.h"
#include "vapor/analyzer/expressions/boolean.h"
#include "vapor/analyzer/helpers.h"
#include "vapor/analyzer/symbol.h"
#include "vapor/parser/lambda_expression.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    std::unique_ptr<statement> if_statement::_clone_with_replacement(replacements & repl) const
    {
        auto ret = std::unique_ptr<if_statement>(new if_statement(*this));

        ret->_condition = repl.claim(_condition.get());

        ret->_then_block = repl.claim(_then_block.get());
        ret->_else_block = fmap(_else_block, [&](auto && block) { return repl.claim(block.get()); });

        return ret;
    }

    future<statement *> if_statement::_simplify(recursive_context ctx)
    {
        auto future = _condition->simplify_expr(ctx)
                          .then([&, ctx](auto && simplified) { replace_uptr(_condition, simplified, ctx.proper); })
                          .then([&, ctx] { return _then_block->simplify(ctx); })
                          .then([&, ctx](auto && simpl) { replace_uptr(_then_block, simpl, ctx.proper); });

        if (_else_block)
        {
            future = future.then([&, ctx] { return _else_block.get()->simplify(ctx); }).then([&, ctx](auto && simpl) {
                replace_uptr(_else_block.get(), simpl, ctx.proper);
            });
        }

        return future.then([&]() -> statement * {
            if (_condition->is_constant())
            {
                if (_condition->get_type() != builtin_types().boolean.get())
                {
                    assert(0);
                }

                auto condition = _condition->as<boolean_constant>()->get_value();
                if (condition)
                {
                    return _then_block.release();
                }

                else
                {
                    if (_else_block)
                    {
                        return _else_block.get().release();
                    }

                    return make_null_statement().release();
                }
            }

            return this;
        });
    }
}
}
