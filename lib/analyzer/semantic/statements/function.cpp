/**
 * Vapor Compiler Licence
 *
 * Copyright © 2016-2019 Michał "Griwes" Dominiak
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
#include "vapor/analyzer/helpers.h"
#include "vapor/analyzer/semantic/function.h"
#include "vapor/analyzer/statements/block.h"
#include "vapor/analyzer/statements/function.h"
#include "vapor/codegen/ir/function.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    future<> function_declaration::_analyze(analysis_context & ctx)
    {
        _function = make_function("overloadable function", get_ast_info().value().range);
        _function->set_name(U"call");
        _function->make_member();
        _function->set_scopes_generator([this](auto && ctx) { return this->_overload_set->get_type()->codegen_scopes(ctx); });

        _overload_set->add_function(this);

        auto initial_future = [&] {
            if (!_return_type)
            {
                return make_ready_future();
            }

            return (*_return_type)->analyze(ctx).then([&] {
                if (!_template_params)
                {
                    auto & type_expr = *_return_type;
                    assert(type_expr->get_type() == builtin_types().type.get());
                    assert(type_expr->is_constant());
                }

                _function->set_return_type(_return_type->get()->_get_replacement());
            });
        }();

        return initial_future.then([&] { return when_all(fmap(_parameter_list, [&](auto && param) { return param->analyze(ctx); })); }).then([&] {
            auto params = fmap(_parameter_list, [](auto && param) -> expression * { return param.get(); });
            params.insert(params.begin(), _overload_set.get());

            _function->set_parameters(std::move(params));
        });
    }

    future<> function_definition::_analyze(analysis_context & ctx)
    {
        auto base_future = function_declaration::_analyze(ctx);

        if (_template_params)
        {
            // FIXME: this is actually wrong
            // the way this here is done would actually implement C++-like templates in typeclass functions
            // that is not what I want; what I want is to only use things that are known from the constraints on the template parameters
            // but this is significantly harder to implement and will have to wait for a while
            // (for the time being the compiler will just be a lot more permissive, and with a future release some code will become invalid,
            // which is fine, because nowhere does Vapor promise being stable for now)
            return base_future;
        }

        return base_future
            .then([&] {
                _function->set_name(U"call");
                _function->set_codegen([=](ir_generation_context & ctx) {
                    auto ret = codegen::ir::function{ U"call",
                        {},
                        fmap(_parameter_list,
                            [&](auto && param) { return std::get<std::shared_ptr<codegen::ir::variable>>(param->codegen_ir(ctx).back().result); }),
                        _body->codegen_return(ctx),
                        _body->codegen_ir(ctx) };
                    ret.is_member = true;
                    return ret;
                });

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
