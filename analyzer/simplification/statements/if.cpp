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

        auto then = repl.claim(_then_block.get()).release();
        ret->_then_block.reset(static_cast<block *>(then));

        fmap(_else_block, [&](auto && block) {
            auto else_ = repl.claim(block.get()).release();
            ret->_else_block = std::unique_ptr<class block>(static_cast<class block *>(else_));
            return unit{};
        });

        return ret;
    }

    future<statement *> if_statement::_simplify(simplification_context & ctx)
    {
        return _condition->simplify_expr(ctx).then([&](auto && simplified) {
            replace_uptr(_condition, simplified, ctx);

            if (_condition->is_constant())
            {
                if (_condition->get_type() != builtin_types().boolean.get())
                {
                    assert(0);
                }

                auto simplify_block = [&](auto && block) {
                    auto released = block.release();
                    return released->simplify(ctx).then([released, &ctx](auto && simpl) {
                        if (released != simpl)
                        {
                            ctx.keep_alive(released);
                        }
                        return simpl;
                    });
                };

                auto condition = _condition->as<boolean_constant>()->get_value();
                if (condition)
                {
                    return simplify_block(_then_block);
                }

                else
                {
                    if (_else_block)
                    {
                        return simplify_block(_else_block.get());
                    }

                    return make_ready_future<statement *>(make_null_statement().release());
                }
            }

            return make_ready_future<statement *>(this);
        });
    }
}
}
