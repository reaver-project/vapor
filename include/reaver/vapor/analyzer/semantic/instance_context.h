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

#pragma once

#include <string>

#include "../expressions/expression.h"
#include "../scope.h"
#include "../simplification/replacements.h"
#include "../symbol.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    class scope;
    class function;

    struct instance_context
    {
        const scope * tc_scope;
        const std::vector<expression *> & arguments;

        auto get_replacements() const
        {
            replacements repl;

            auto && params = tc_scope->parent()->symbols_in_order();

            assert(arguments.size() == params.size());
            assert(std::equal(arguments.begin(), arguments.end(), params.begin(), [](auto && lhs, auto && rhs) { return lhs->get_type() == rhs->get_type(); }));

            for (std::size_t i = 0; i < arguments.size(); ++i)
            {
                repl.add_replacement(params[i]->get_expression(), arguments[i]);
            }

            return repl;
        };
    };

    struct instance_function_context
    {
        const function * original_overload;
    };
}
}

namespace reaver::vapor::parser
{
inline namespace _v1
{
    class function_definition;
}
}

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    using function_definition_handler = reaver::unique_function<void(precontext & ctx, const parser::function_definition &)>;
}
}
