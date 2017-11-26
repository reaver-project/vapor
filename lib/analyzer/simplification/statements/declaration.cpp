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

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    std::unique_ptr<statement> declaration::_clone_with_replacement(replacements & repl) const
    {
        assert(_type == declaration_type::variable);
        return repl.claim(_init_expr.value().get());
    }

    future<statement *> declaration::_simplify(recursive_context ctx)
    {
        auto fut = make_ready_future<statement *>(this);

        fmap(_init_expr, [&](auto && expr) {
            fut = expr->simplify_expr(ctx)
                      .then([&, ctx](auto && simplified) {
                          replace_uptr(_init_expr.value(), simplified, ctx.proper);
                          return _declared_symbol->simplify(ctx);
                      })
                      .then([&]() -> statement * { return _init_expr->release(); });

            return unit{};
        });

        return fut;
    }
}
}
