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

#include "vapor/analyzer/expressions/instance.h"
#include "vapor/analyzer/expressions/expression_list.h"
#include "vapor/analyzer/statements/function.h"
#include "vapor/analyzer/symbol.h"
#include "vapor/parser/typeclass.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    std::unique_ptr<instance_literal> preanalyze_instance_literal(precontext & ctx, const parser::instance_literal & parse, scope * lex_scope)
    {
        auto name_id_expr = fmap(parse.typeclass_name.id_expression_value, [&](auto && token) { return token.value.string; });

        auto late_preanalysis = [&parse, &ctx](scope * lex_scope, const scope * typeclass_scope, const std::vector<expression *> arguments) {
            return fmap(parse.definitions, [&](auto && definition) {
                return std::get<0>(fmap(definition, make_overload_set([&](const parser::function_definition & func) -> std::unique_ptr<statement> {
                    return preanalyze_function_definition(ctx, func, lex_scope, instance_context{ typeclass_scope, arguments });
                })));
            });
        };

        return std::make_unique<instance_literal>(make_node(parse),
            lex_scope,
            std::move(name_id_expr),
            fmap(parse.arguments.expressions, [&](auto && expr) { return preanalyze_expression(ctx, expr, lex_scope); }),
            std::move(late_preanalysis));
    }

    instance_literal::instance_literal(ast_node parse,
        scope * original_scope,
        std::vector<std::u32string> name_segments,
        std::vector<std::unique_ptr<expression>> arguments,
        late_preanalysis_type late_pre)
        : _original_scope{ original_scope },
          _typeclass_name{ std::move(name_segments) },
          _arguments{ std::move(arguments) },
          _late_preanalysis{ std::move(late_pre) }
    {
        _set_ast_info(parse);
    }

    void instance_literal::print(std::ostream & os, print_context ctx) const
    {
        assert(0);
    }

    statement_ir instance_literal::_codegen_ir(ir_generation_context & ctx) const
    {
        assert(0);
    }
}
}
