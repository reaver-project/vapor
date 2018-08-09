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

#include "vapor/analyzer/expressions/template.h"
#include "vapor/analyzer/expressions/typeclass.h"
#include "vapor/analyzer/symbol.h"
#include "vapor/parser/expr.h"
#include "vapor/parser/template.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    std::unique_ptr<template_expression> preanalyze_template_expression(precontext & ctx, const parser::template_expression & parse, scope * lex_scope)
    {
        auto scope = lex_scope->clone_for_class();
        auto scope_ptr = scope.get();
        auto ret = std::make_unique<template_expression>(make_node(parse),
            std::move(scope),
            preanalyze_parameter_list(ctx, parse.parameters.template_parameters, scope_ptr),
            std::get<0>(fmap(parse.expression, make_overload_set([&](const parser::typeclass_literal & typeclass) -> std::unique_ptr<expression> {
                return preanalyze_typeclass_literal(ctx, typeclass, scope_ptr);
            }))));
        scope_ptr->close();
        return ret;
    }

    template_expression::template_expression(ast_node parse,
        std::unique_ptr<scope> template_scope,
        parameter_list params,
        std::unique_ptr<expression> templated_expr)
        : _scope{ std::move(template_scope) }, _params{ std::move(params) }, _templated_expression{ std::move(templated_expr) }
    {
        _set_ast_info(parse);
    }

    void template_expression::print(std::ostream & os, print_context ctx) const
    {
        os << styles::def << ctx << styles::rule_name << "template-expression";
        print_address_range(os, this);
        os << '\n';

        auto params_ctx = ctx.make_branch(false);
        os << styles::def << params_ctx << styles::subrule_name << "template parameters:\n";

        std::size_t idx = 0;
        for (auto && param : _params)
        {
            param->print(os, params_ctx.make_branch(++idx == _params.size()));
        }

        auto subexpr_ctx = ctx.make_branch(true);
        os << styles::def << subexpr_ctx << styles::subrule_name << "templated expression:\n";
        _templated_expression->print(os, subexpr_ctx.make_branch(true));
    }

    statement_ir template_expression::_codegen_ir(ir_generation_context & ctx) const
    {
        assert(0);
    }
}
}
