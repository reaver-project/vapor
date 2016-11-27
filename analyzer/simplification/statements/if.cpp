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

#include "vapor/analyzer/statements/if.h"
#include "vapor/analyzer/helpers.h"
#include "vapor/analyzer/symbol.h"
#include "vapor/analyzer/variables/boolean.h"
#include "vapor/parser/lambda_expression.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    std::unique_ptr<statement> if_statement::_clone_with_replacement(replacements & repl) const
    {
        auto ret = std::unique_ptr<if_statement>(new if_statement(*this));

        ret->_condition = _condition->clone_expr_with_replacement(repl);

        auto then = _then_block->clone_with_replacement(repl).release();
        ret->_then_block.reset(static_cast<block *>(then));

        fmap(_else_block, [&](auto && block) {
            auto else_ = block->clone_with_replacement(repl).release();
            ret->_else_block = std::unique_ptr<class block>(static_cast<class block *>(else_));
            return unit{};
        });

        return ret;
    }

    future<statement *> if_statement::_simplify(simplification_context & ctx)
    {
        return _condition->simplify_expr(ctx).then([&](auto && simplified) {
            replace_uptr(_condition, simplified, ctx);

            auto var = _condition->get_variable();
            if (var->is_constant())
            {
                if (var->get_type() != builtin_types().boolean.get())
                {
                    assert(0);
                }

                auto condition = dynamic_cast<boolean_constant *>(var)->get_value();
                if (condition)
                {
                    return _then_block.release()->simplify(ctx);
                }

                else
                {
                    if (_else_block)
                    {
                        return _else_block.get().release()->simplify(ctx);
                    }

                    return make_null_statement().release()->simplify(ctx);
                }
            }

            return make_ready_future<statement *>(this);
        });
    }
}
}
