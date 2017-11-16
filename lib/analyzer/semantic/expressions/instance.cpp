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

        return expr.then([&](expression * expr) { return expr->analyze(ctx).then([expr] { return expr; }); })
            .then([&](expression * expr) {
                auto typeclass = expr->_get_replacement()->as<typeclass_literal>();
                assert(typeclass);

                _typeclass_scope = typeclass->get_scope();
                assert(_typeclass_scope);

                _scope = _original_scope->combine_with(_typeclass_scope);
            })
            .then([&] { return when_all(fmap(_arguments, [&](auto && arg) { return arg->analyze(ctx); })); })
            .then([&] {
                _definitions = _late_preanalysis(_scope.get(), _typeclass_scope, fmap(_arguments, [](auto && arg) { return arg.get(); }));

                return when_all(fmap(_definitions, [&](auto && stmt) { return stmt->analyze(ctx); }));
            });
    }
}
}
