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

#include "vapor/analyzer/types/type.h"
#include "vapor/analyzer/function.h"
#include "vapor/analyzer/symbol.h"
#include "vapor/analyzer/variables/type.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    void type_type::_codegen_type(ir_generation_context &) const
    {
        assert(0);
    }

    future<function *> resolve_overload(const variable * lhs, const variable * rhs, lexer::token_type op, scope * in_scope)
    {
        return lhs->get_type()->get_overload(op, rhs).then([](auto && overload) {
            assert(overload);
            return overload;
        });
    }

    future<function *> resolve_overload(const variable * base_expr, lexer::token_type bracket_type, std::vector<const variable *> arguments, scope * in_scope)
    {
        return base_expr->get_type()->get_overload(bracket_type, base_expr, std::move(arguments)).then([](auto && overload) {
            assert(overload);
            return overload;
        });
    }

    future<function *> type_type::get_overload(lexer::token_type token, const variable * base, std::vector<const variable *> args) const
    {
        if (token != lexer::token_type::curly_bracket_open)
        {
            return make_ready_future<function *>(nullptr);
        }

        assert(base->is_constant());
        assert(base->get_type() == builtin_types().type.get()); // this check is a little paranoid

        auto base_type = static_cast<const type_variable *>(base)->get_value();
        return base_type->get_constructor(args);
    }
}
}
