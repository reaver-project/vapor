/**
 * Vapor Compiler Licence
 *
 * Copyright © 2017 Michał "Griwes" Dominiak
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

#include "vapor/parser/parameter_list.h"
#include "vapor/analyzer/semantic/parameter_list.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    parameter::parameter(ast_node parse, std::u32string name, std::unique_ptr<expression> type) : _name{ std::move(name) }, _type_expression{ std::move(type) }
    {
        _set_ast_info(parse);
    }

    parameter_list preanalyze_parameter_list(const parser::parameter_list & param_list, scope * lex_scope)
    {
        return fmap(param_list.parameters, [&](auto && param_parse) {
            assert(param_parse.type);

            auto param =
                std::make_unique<parameter>(make_node(param_parse), param_parse.name.value.string, preanalyze_expression(param_parse.type.value(), lex_scope));

            auto symb = make_symbol(param_parse.name.value.string, param.get());
            lex_scope->init(param_parse.name.value.string, std::move(symb));

            return param;
        });
    }
}
}
