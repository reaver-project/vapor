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

    future<std::vector<function *>> type_type::get_candidates(lexer::token_type token) const
    {
        if (token != lexer::token_type::curly_bracket_open)
        {
            return make_ready_future(std::vector<function *>{});
        }

        [&] {
            std::lock_guard<std::mutex> lock{ _generic_ctor_lock };

            if (_generic_ctor)
            {
                return;
            }

            _generic_ctor_first_arg = make_blank_variable(builtin_types().type.get());
            // _generic_ctor_pack_arg = make_blank_pack_variable();

            _generic_ctor = make_function("generic constructor",
                builtin_types().type->get_expression(),
                { _generic_ctor_first_arg.get(), _generic_ctor_pack_arg.get() },
                [](auto &&) -> codegen::ir::function { assert(!"tried to codegen the generic constructor!"); });

            // _generic_ctor->set_return_type_expression(_generic_ctor_first_arg.get());

            _generic_ctor->set_eval([](auto && ctx, auto && args) -> future<expression *> {
                assert(args.size() != 0);
                assert(args.front()->get_type() == builtin_types().type.get());

                auto type_var = static_cast<type_variable *>(args.front());
                auto actual_type = type_var->get_value();
                args.erase(args.begin());
                auto actual_ctor = actual_type->get_constructor(fmap(args, [](auto && arg) -> const variable * { return arg; }));

                return actual_ctor.then([args](auto && ctor) -> expression * { return make_call_expression(ctor, args).release(); });
            });
        }();

        return make_ready_future(std::vector<function *>{ _generic_ctor.get() });
    }
}
}
