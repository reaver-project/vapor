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
    future<> binary_expression::_analyze(analysis_context & ctx)
    {
        return when_all(_lhs->analyze(ctx), _rhs->analyze(ctx))
            .then([&](auto) { return resolve_overload(_lhs->get_type(), _rhs->get_type(), _op.type, _scope); })
            .then([&](auto && overload) {
                _overload = overload;
                return _overload->return_type();
            })
            .then([&](auto && ret_type) { this->_set_variable(make_expression_variable(this, ret_type)); });
    }
}
}
