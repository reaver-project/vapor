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

#include "vapor/analyzer/expressions/expression.h"
#include "vapor/analyzer/scope.h"
#include "vapor/analyzer/symbol.h"
#include "vapor/analyzer/types/type.h"
#include "vapor/analyzer/variables/expression.h"
#include "vapor/analyzer/variables/member.h"
#include "vapor/analyzer/variables/variable.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    variable_ir expression_variable::_codegen_ir(ir_generation_context & ctx) const
    {
        return { _expression->codegen_ir(ctx).back().result };
    }

    future<variable *> expression_variable::_simplify(simplification_context & ctx)
    {
        return _expression->simplify_expr(ctx).then([&](auto && simplified) -> variable * { return simplified->get_variable(); });
    }

    std::unique_ptr<variable> expression_variable::_clone_with_replacement(replacements & repl) const
    {
        auto cloned_expr = _expression->clone_expr_with_replacement(repl);
        repl.expressions[_expression.get()] = cloned_expr.get();
        return make_expression_variable(std::move(cloned_expr), _type);
    }

    bool expression_variable::is_constant() const
    {
        return false;
    }

    bool expression_variable::is_equal(const variable * ptr) const
    {
        return false;
    }

    variable_ir expression_ref_variable::_codegen_ir(ir_generation_context & ctx) const
    {
        return { _expression->codegen_ir(ctx).back().result };
    }

    future<variable *> expression_ref_variable::_simplify(simplification_context & ctx)
    {
        return _expression->simplify_expr(ctx).then([&](auto && simplified) -> variable * { return simplified->get_variable(); });
    }

    std::unique_ptr<variable> expression_ref_variable::_clone_with_replacement(replacements & repl) const
    {
        auto it = repl.expressions.find(_expression);
        assert(it != repl.expressions.end());

        return make_expression_ref_variable(it->second, _type);
    }

    bool expression_ref_variable::is_constant() const
    {
        return false;
    }

    bool expression_ref_variable::is_equal(const variable * ptr) const
    {
        return false;
    }
}
}
