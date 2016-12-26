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

#include "vapor/analyzer/expressions/closure.h"
#include "vapor/analyzer/helpers.h"
#include "vapor/analyzer/symbol.h"
#include "vapor/analyzer/types/closure.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    future<> closure::_analyze(analysis_context & ctx)
    {
        auto initial_future = [&] {
            if (_return_type)
            {
                return (*_return_type)->analyze(ctx).then([&] {
                    auto var = (*_return_type)->get_variable();

                    assert(var->get_type() == builtin_types().type.get());
                    assert(var->is_constant());
                });
            }

            return make_ready_future();
        }();

        return initial_future.then([&] { return when_all(fmap(_argument_list, [&](auto && arg) { return arg.type_expression->analyze(ctx); })); })
            .then([&] {
                fmap(_argument_list, [&](auto && arg) {
                    arg.variable->set_type(arg.type_expression->get_variable());
                    return unit{};
                });

                return _body->analyze(ctx);
            })
            .then([&] {
                fmap(_return_type, [&](auto && ret_type) {
                    auto explicit_type = ret_type->get_variable();
                    auto type_var = dynamic_cast<type_variable *>(explicit_type);

                    assert(type_var->get_value() == _body->return_type());

                    return unit{};
                });

                auto arg_variables = fmap(_argument_list, [&](auto && arg) -> variable * { return arg.variable.get(); });

                auto function = make_function("closure",
                    _body->return_type(),
                    std::move(arg_variables),
                    [this](ir_generation_context & ctx) {
                        auto ret = codegen::ir::function{
                            U"operator()",
                            {},
                            fmap(_argument_list,
                                [&](auto && arg) {
                                    return get<std::shared_ptr<codegen::ir::variable>>(get<codegen::ir::value>(arg.variable->codegen_ir(ctx)));
                                }),
                            _body->codegen_return(ctx),
                            _body->codegen_ir(ctx),
                        };
                        ret.is_member = true;
                        return ret;
                    },
                    _parse.range);

                function->set_name(U"operator()");
                function->set_body(_body.get());

                _type = std::make_unique<closure_type>(_scope.get(), this, std::move(function));
                _set_variable(make_expression_variable(this, _type.get()));
            });
    }
}
}
