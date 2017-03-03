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

#include "vapor/analyzer/variables/variable.h"
#include "vapor/analyzer/expressions/expression.h"
#include "vapor/analyzer/scope.h"
#include "vapor/analyzer/symbol.h"
#include "vapor/analyzer/types/type.h"
#include "vapor/analyzer/variables/member.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    variable * variable::get_member(const member_variable * member) const
    {
        return get_member(member->get_name());
    }

    variable * variable::get_member(const std::u32string & name) const
    {
        if (auto symbol = get_type()->get_scope()->try_get(name))
        {
            return symbol.get()->get_variable();
        }

        return nullptr;
    }

    void variable::set_default_value(const expression * expr)
    {
        assert(!_default_value);
        assert(expr);
        assert(expr->get_type() == get_type());
        _default_value = expr;
    }

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
        auto it = repl.expressions.find(_expression);
        assert(it != repl.expressions.end());

        return make_expression_variable(it->second, _type);
    }

    bool expression_variable::is_constant() const
    {
        return false;
    }

    bool expression_variable::is_equal(const variable * ptr) const
    {
        return false;
    }
}
}
