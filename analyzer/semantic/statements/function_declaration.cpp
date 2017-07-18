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

#include <numeric>

#include "vapor/analyzer/expressions/overload_set.h"
#include "vapor/analyzer/expressions/type.h"
#include "vapor/analyzer/function.h"
#include "vapor/analyzer/helpers.h"
#include "vapor/analyzer/statements/block.h"
#include "vapor/analyzer/statements/function_declaration.h"
#include "vapor/codegen/ir/function.h"
#include "vapor/parser/lambda_expression.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    future<> function_declaration::_analyze(analysis_context & ctx)
    {
        _function = make_function("overloadable function",
            nullptr,
            {},
            [=, name = _parse.name.value.string](ir_generation_context & ctx) {
                auto ret = codegen::ir::function{ U"operator()",
                    {},
                    fmap(_parameter_list, [&](auto && param) { return get<std::shared_ptr<codegen::ir::variable>>(param->codegen_ir(ctx).back().result); }),
                    _body->codegen_return(ctx),
                    _body->codegen_ir(ctx) };
                ret.is_member = true;
                return ret;
            },
            _parse.range);
        _function->set_name(U"operator()");
        _function->make_member();
        _overload_set->add_function(this);

        auto initial_future = [&] {
            if (_return_type)
            {
                return (*_return_type)->analyze(ctx).then([&] {
                    auto & type_expr = *_return_type;

                    assert(type_expr->get_type() == builtin_types().type.get());
                    assert(type_expr->is_constant());

                    _function->set_return_type(_return_type->get());
                });
            }

            return make_ready_future();
        }();

        return initial_future.then([&] { return when_all(fmap(_parameter_list, [&](auto && param) { return param->analyze(ctx); })); })
            .then([&] {
                auto params = fmap(_parameter_list, [](auto && param) -> expression * { return param.get(); });
                params.insert(params.begin(), _overload_set.get());

                _function->set_parameters(std::move(params));

                return _body->analyze(ctx);
            })
            .then([&] {
                fmap(_return_type, [&](auto && ret_type) {
                    auto type_expr = ret_type->template as<type_expression>();
                    assert(type_expr->get_value() == _body->return_type());

                    return unit{};
                });

                _function->set_body(_body.get());

                if (!_return_type)
                {
                    _function->set_return_type(make_type_expression(_body->return_type()));
                }
            });
    }
}
}
