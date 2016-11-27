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
#include "vapor/analyzer/expressions/id.h"
#include "vapor/analyzer/expressions/postfix.h"
#include "vapor/analyzer/function.h"
#include "vapor/analyzer/helpers.h"
#include "vapor/parser.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    future<> postfix_expression::_analyze(analysis_context & ctx)
    {
        return when_all(fmap(_arguments, [&](auto && expr) { return expr->analyze(ctx); })).then([&] { return _base_expr->analyze(ctx); }).then([&] {
            if (!_parse.bracket_type)
            {
                return make_ready_future();
            }

            return resolve_overload(
                _base_expr->get_type(), *_parse.bracket_type, fmap(_arguments, [](auto && arg) -> const type * { return arg->get_type(); }), _scope)
                .then([&](auto && overload) {
                    _overload = overload;
                    return _overload->return_type();
                })
                .then([&](auto && ret_type) { this->_set_variable(make_expression_variable(this, ret_type)); });
        });
    }
}
}
