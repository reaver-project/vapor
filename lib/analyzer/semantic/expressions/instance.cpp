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
#include "vapor/analyzer/symbol.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    future<> instance_literal::_analyze(analysis_context & ctx)
    {
        return foldl(_typeclass_name,
            make_ready_future<scope *>(+_original_scope),
            [](future<scope *> lex_scope, auto && name) {
                return lex_scope.then([name](scope * lex_scope) { return lex_scope->get(name)->get_expression_future(); }).then([](auto && expr) {
                    return expr->get_type()->get_scope();
                });
            })
            .then([&](scope * typeclass_scope) {
                assert(0);
                // combine scopes
                _definitions = _late_preanalysis(_combined_scopes.get());
            });
    }
}
}
