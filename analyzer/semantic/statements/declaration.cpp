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

#include "vapor/analyzer/statements/declaration.h"
#include "vapor/analyzer/expressions/member.h"
#include "vapor/analyzer/expressions/type.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    future<> declaration::_analyze(analysis_context & ctx)
    {
        auto fut = make_ready_future();

        fmap(_type_specifier, [&](auto && expr) {
            fut = fut.then([&]() { return expr->analyze(ctx); }).then([&]() {
                auto && type_expr = _type_specifier.get();
                assert(type_expr->get_type() == builtin_types().type.get());
                assert(type_expr->is_constant());
            });

            return unit{};
        });

        fmap(_init_expr, [&](auto && expr) {
            fut = fut.then([&]() { return expr->analyze(ctx); });

            fmap(_type_specifier, [&](auto && expr) {
                fut = fut.then([&]() {
                    auto type_var = expr->template as<type_expression>();
                    assert(_init_expr.get()->get_type() == type_var->get_value());
                });

                return unit{};
            });

            fut = fut.then([&] {
                auto expression = _init_expr.get().get();

                if (_type == declaration_type::member)
                {
                    _declared_member = make_member_expression(nullptr, _parse.identifier.value.string, _init_expr.get()->get_type());
                    _declared_member.get()->set_default_value(_init_expr.get().get());
                    expression = _declared_member.get().get();
                }

                _declared_symbol->set_expression(expression);
            });

            return unit{};
        });

        if (!_init_expr)
        {
            fut = fut.then([&]() {
                assert(_type_specifier);
                auto type = _type_specifier.get()->as<type_expression>()->get_value();
                _declared_member = make_member_expression(nullptr, _parse.identifier.value.string, type);

                _declared_symbol->set_expression(_declared_member.get().get());
            });
        }

        return fut;
    }
}
}
