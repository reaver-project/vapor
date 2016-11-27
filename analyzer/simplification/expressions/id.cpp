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

#include "vapor/analyzer/expressions/id.h"
#include "vapor/analyzer/expressions/variable.h"
#include "vapor/parser.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    std::unique_ptr<expression> id_expression::_clone_expr_with_replacement(replacements & repl) const
    {
        auto referenced = _referenced;

        auto it = repl.variables.find(referenced);
        if (it != repl.variables.end())
        {
            referenced = it->second;
        }

        return make_variable_ref_expression(referenced);
    }

    reaver::future<expression *> id_expression::_simplify_expr(simplification_context & ctx)
    {
        return _referenced->simplify(ctx).then([&](auto && simplified) -> expression * {
            if (simplified && simplified != _referenced)
            {
                _referenced = simplified;
            }
            return this;
        });
    }
}
}
