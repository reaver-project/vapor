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

#pragma once

#include "../../parser/parameter_list.h"
#include "../expressions/expression.h"
#include "../symbol.h"
#include "../variables/unresolved.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    struct parameter
    {
        std::u32string name;
        std::unique_ptr<expression> type_expression;
        std::unique_ptr<unresolved_variable> variable;
    };

    using parameter_list = std::vector<parameter>;

    inline parameter_list preanalyze_parameter_list(const parser::parameter_list & arglist, scope * lex_scope)
    {
        return fmap(arglist.parameters, [&](auto && arg) {
            auto expr = preanalyze_expression(arg.type, lex_scope);
            auto var = make_unresolved_variable(arg.name.string);
            var->mark_local();

            auto symb = make_symbol(arg.name.string, var.get());
            lex_scope->init(arg.name.string, std::move(symb));

            return parameter{ arg.name.string, std::move(expr), std::move(var) };
        });
    }
}
}
