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

#include <boost/type_index.hpp>

#include "vapor/analyzer/expressions/binary.h"
#include "vapor/analyzer/function.h"
#include "vapor/analyzer/helpers.h"
#include "vapor/analyzer/symbol.h"
#include "vapor/parser.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    std::unique_ptr<expression> binary_expression::_clone_expr_with_replacement(replacements & repl) const
    {
        auto ret = std::unique_ptr<binary_expression>(new binary_expression(*this));

        ret->_lhs = _lhs->clone_expr_with_replacement(repl);
        ret->_rhs = _rhs->clone_expr_with_replacement(repl);

        return ret;
    }

    future<expression *> binary_expression::_simplify_expr(simplification_context & ctx)
    {
        return when_all(_lhs->simplify_expr(ctx), _rhs->simplify_expr(ctx))
            .then([&](auto && simplified) {
                replace_uptr(_lhs, get<0>(simplified), ctx);
                replace_uptr(_rhs, get<1>(simplified), ctx);
                return _overload->simplify(ctx, { _lhs->get_variable(), _rhs->get_variable() });
            })
            .then([&](auto && simplified) -> expression * {
                if (simplified)
                {
                    return simplified;
                }

                return this;
            });
    }
}
}
