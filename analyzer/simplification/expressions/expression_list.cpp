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
#include "vapor/analyzer/helpers.h"
#include "vapor/analyzer/symbol.h"
#include "vapor/parser/expression_list.h"
#include "vapor/parser/lambda_expression.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    future<expression *> expression_list::_simplify_expr(simplification_context & ctx)
    {
        return when_all(fmap(value, [&](auto && expr) { return expr->simplify_expr(ctx); })).then([&](auto && simplified) -> expression * {
            replace_uptrs(value, simplified, ctx);
            assert(0);
            return this;
        });
    }

    std::unique_ptr<expression> expression_list::_clone_expr_with_replacement(replacements & repl) const
    {
        auto ret = std::make_unique<expression_list>();
        ret->range = range;

        ret->value = fmap(value, [&](auto && expr) { return expr->clone_expr_with_replacement(repl); });

        return ret;
    }
}
}
