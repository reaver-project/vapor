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

#include <boost/type_index.hpp>

#include "vapor/analyzer/expressions/binary.h"
#include "vapor/analyzer/expressions/call.h"
#include "vapor/analyzer/function.h"
#include "vapor/analyzer/helpers.h"
#include "vapor/analyzer/semantic/overloads.h"
#include "vapor/analyzer/symbol.h"
#include "vapor/parser.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    future<> binary_expression::_analyze(analysis_context & ctx)
    {
        auto expr_ctx = get_context();
        expr_ctx.push_back(this);

        _lhs->set_context(expr_ctx);
        _rhs->set_context(expr_ctx);

        return when_all(_lhs->analyze(ctx), _rhs->analyze(ctx))
            .then([&](auto) { return resolve_overload(ctx, _parse.range, _lhs.get(), _rhs.get(), _op.type); })
            .then([&](std::unique_ptr<expression> call_expr) {
                if (auto call_expr_downcasted = call_expr->as<call_expression>())
                {
                    call_expr_downcasted->set_parse_range(_parse.range);
                }
                _call_expression = std::move(call_expr);
                return _call_expression->analyze(ctx);
            });
    }
}
}
