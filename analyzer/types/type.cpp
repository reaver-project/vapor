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

#include "vapor/analyzer/types/type.h"
#include "vapor/analyzer/expressions/call.h"
#include "vapor/analyzer/expressions/variable.h"
#include "vapor/analyzer/function.h"
#include "vapor/analyzer/symbol.h"
#include "vapor/analyzer/variables/type.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    type::~type() = default;

    expression * type::get_expression()
    {
        _self_expression = make_variable_expression(make_type_variable(this));
        return _self_expression.get();
    }

    void type_type::_codegen_type(ir_generation_context &) const
    {
        assert(0);
    }

    future<std::unique_ptr<expression>> resolve_overload(const expression * lhs, const expression * rhs, lexer::token_type op, scope * in_scope)
    {
        return lhs->get_type()->get_candidates(op).then([lhs, rhs](auto && overloads) -> std::unique_ptr<expression> {
            assert(0);
            // assert(overload);
            // return make_call_expression(overload, { lhs, rhs });
        });
    }

    future<std::unique_ptr<expression>> resolve_overload(const expression * base_expr,
        lexer::token_type bracket_type,
        std::vector<const expression *> arguments,
        scope * in_scope)
    {
        return base_expr->get_type()->get_candidates(bracket_type).then([arguments](auto && overloads) -> std::unique_ptr<expression> {
            assert(0);
            // assert(overload);
            // return make_call_expression(overload, std::move(arguments));
        });
    }

    future<std::vector<function *>> type_type::get_candidates(lexer::token_type token) const
    {
        assert(0);

        if (token != lexer::token_type::curly_bracket_open)
        {
            return make_ready_future(std::vector<function *>{});
        }

        /*assert(base->is_constant());
        assert(base->get_type() == builtin_types().type.get()); // this check is a little paranoid

        auto base_type = static_cast<const type_variable *>(base)->get_value();
        return base_type->get_constructor(args);*/
    }
}
}
