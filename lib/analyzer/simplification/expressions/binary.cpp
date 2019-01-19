/**
 * Vapor Compiler Licence
 *
 * Copyright © 2016-2017, 2019 Michał "Griwes" Dominiak
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

#include <boost/type_index.hpp>

#include "vapor/analyzer/expressions/binary.h"
#include "vapor/analyzer/helpers.h"
#include "vapor/analyzer/semantic/function.h"
#include "vapor/analyzer/semantic/symbol.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    std::unique_ptr<expression> binary_expression::_clone_expr_with_replacement(replacements & repl) const
    {
        return repl.claim(_call_expression.get());
    }

    future<expression *> binary_expression::_simplify_expr(recursive_context ctx)
    {
        replacements repl;
        auto clone = _clone_expr_with_replacement(repl).release();

        return clone->simplify_expr(ctx).then([ctx, clone](auto && simplified) {
            if (simplified)
            {
                ctx.proper.keep_alive(clone);
                return simplified;
            }
            return clone;
        });
    }
}
}
