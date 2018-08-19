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

                auto instance_scope = _original_scope->clone_for_class();

                std::vector<std::shared_ptr<overload_set>> osets;

                for (auto && oset_name : instance_type_expr->instance_type()->overload_set_names())
                {
                    auto type_name = U"overload_set_types$" + oset_name;

                    auto oset = std::make_shared<overload_set>(instance_scope.get());
                    instance_scope->init(oset_name, make_symbol(oset_name, oset.get()));

                    auto type = oset->get_type();
                    type->set_name(type_name);
                    instance_scope->init(type_name, make_symbol(type_name, type->get_expression()));

                    osets.push_back(std::move(oset));
                }

                assert(0);

                instance_scope->close();

                return when_all(fmap(_definitions, [&](auto && stmt) { return stmt->analyze(ctx); }));
            });
    }
}
}
