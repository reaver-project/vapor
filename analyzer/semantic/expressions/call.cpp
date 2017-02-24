/**
 * Vapor Compiler Licence
 *
 * Copyright © 2017 Michał "Griwes" Dominiak
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

#include "vapor/analyzer/expressions/call.h"
#include "vapor/analyzer/symbol.h"
#include "vapor/analyzer/variables/type.h"
#include "vapor/analyzer/variables/variable.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    future<> call_expression::_analyze(analysis_context &)
    {
        return _function->get_return_type().then([&](auto && type_expr) {
            auto var = type_expr->get_variable();
            assert(var->get_type() == builtin_types().type.get());
            auto type_var = static_cast<type_variable *>(var);

            _var = make_blank_variable(type_var->get_value());
        });
    }
}
}
