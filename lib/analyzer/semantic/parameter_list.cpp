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

#include "vapor/parser/parameter_list.h"
#include "vapor/analyzer/expressions/overload_set.h"
#include "vapor/analyzer/function.h"
#include "vapor/analyzer/semantic/parameter_list.h"
#include "vapor/parser/expr.h"
#include "vapor/parser/parameter_list.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    parameter::parameter(ast_node parse, std::u32string name, std::unique_ptr<expression> type) : _name{ std::move(name) }, _type_expression{ std::move(type) }
    {
        _set_ast_info(parse);
    }

    parameter_list preanalyze_parameter_list(precontext & prectx,
        const parser::parameter_list & param_list,
        scope * lex_scope,
        std::optional<instance_function_context> ctx)
    {
        // TODO: drop the +1 once overload_set's thingies stop being members
        std::size_t i = 0 + 1;

        return fmap(param_list.parameters, [&](auto && param_parse) {
            assert(param_parse.type || ctx);

            auto type = [&] {
                if (param_parse.type)
                {
                    return preanalyze_expression(prectx, param_parse.type.value(), lex_scope);
                }

                else if (ctx)
                {
                    auto repl = ctx->instance.get_replacements();
                    return repl.claim(ctx->original_overload->parameters()[i]->as<parameter>()->get_type_expression());
                }

                assert(!"a type not provided outside of an instance context");
            }();

            auto param = std::make_unique<parameter>(make_node(param_parse), param_parse.name.value.string, std::move(type));

            auto symb = make_symbol(param_parse.name.value.string, param.get());
            lex_scope->init(param_parse.name.value.string, std::move(symb));

            ++i;
            return param;
        });
    }
}
}
