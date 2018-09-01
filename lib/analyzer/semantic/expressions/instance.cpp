/**
 * Vapor Compiler Licence
 *
 * Copyright © 2017-2018 Michał "Griwes" Dominiak
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

#include <reaver/prelude/fold.h>

#include "vapor/analyzer/expressions/instance.h"
#include "vapor/analyzer/expressions/overload_set.h"
#include "vapor/analyzer/expressions/template.h"
#include "vapor/analyzer/expressions/typeclass.h"
#include "vapor/analyzer/statements/function.h"
#include "vapor/analyzer/symbol.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    future<> instance_literal::_analyze(analysis_context & ctx)
    {
        auto name = _typeclass_name;
        auto top_level = std::move(name.front());
        name.erase(name.begin());

        future<expression *> expr = _original_scope->resolve(top_level)->get_expression_future();

        if (!name.empty())
        {
            expr = foldl(name, std::move(expr), [&ctx](future<expression *> expr, auto && name) {
                return expr.then([&](auto && expr) { return expr->analyze(ctx).then([expr] { return expr->get_type()->get_scope(); }); })
                    .then([name = std::move(name)](const scope * lex_scope) { return lex_scope->get(name)->get_expression_future(); });
            });
        }

        // TODO: this can be refactored to use the typeclass template's call operator
        // I don't think that is *really* necessary, but might lead to a very slight cleanup here
        return expr.then([&](expression * expr) { return expr->analyze(ctx).then([expr] { return expr; }); })
            .then([&](expression * expr) {
                return when_all(fmap(_arguments, [&](auto && arg) { return arg->analyze(ctx).then([&] { return simplification_loop(ctx, arg); }); }))
                    .then([expr](auto &&) { return expr; });
            })
            .then([&](expression * expr) {
                auto tpl = expr->_get_replacement()->as<template_expression>();
                assert(tpl);
                auto instance_type_expr =
                    dynamic_cast<typeclass_literal_instance *>(ctx.get_instantiation(tpl, fmap(_arguments, [](auto && ptr) { return ptr.get(); })));
                assert(instance_type_expr);
                _set_type(instance_type_expr->instance_type());

                _instance_scope = get_type()->get_scope()->clone_for_class();

                std::vector<std::shared_ptr<overload_set>> osets;

                for (auto && oset_name : instance_type_expr->instance_type()->overload_set_names())
                {
                    auto oset = create_overload_set(_instance_scope.get(), oset_name);
                    osets.push_back(std::move(oset));
                }

                // close here, because if the preanalysis below *adds* new members, then we have a bug... the assertion that checks for closeness of the scope
                // needs to be somehow weakened here, to allow for more sensible error reporting than `assert`
                _instance_scope->close();

                _late_preanalysis([&](precontext & ctx, const parser::function_definition & parse) {
                    auto scope = _instance_scope.get();
                    auto func = preanalyze_function_definition(ctx, parse, scope, true);
                    assert(scope == _instance_scope.get());
                    _function_definitions.push_back(std::move(func));
                });

                return when_all(fmap(_function_definitions, [&](auto && fn_def) { return fn_def->analyze(ctx); }));
            })
            .then([&] { assert(0); });
    }
}
}
