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

#include "vapor/analyzer/variable.h"
#include "vapor/analyzer/expression.h"
#include "vapor/analyzer/symbol.h"

reaver::vapor::analyzer::_v1::variable_ir reaver::vapor::analyzer::_v1::expression_variable::_codegen_ir(reaver::vapor::analyzer::_v1::ir_generation_context & ctx) const
{
    return {
        _expression->codegen_ir(ctx).back().result
    };
}

reaver::future<reaver::vapor::analyzer::_v1::variable *> reaver::vapor::analyzer::_v1::expression_variable::_simplify(reaver::vapor::analyzer::_v1::optimization_context & ctx)
{
    return _expression->simplify_expr(ctx)
        .then([&](auto && simplified) -> variable * {
            return simplified->get_variable();
        });
}

bool reaver::vapor::analyzer::_v1::expression_variable::is_constant() const
{
    return false;
}

bool reaver::vapor::analyzer::_v1::expression_variable::is_equal(const reaver::vapor::analyzer::_v1::variable * ptr) const
{
    return false;
}

