/**
 * Vapor Compiler Licence
 *
 * Copyright © 2017-2019 Michał "Griwes" Dominiak
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

#include "vapor/analyzer/expressions/typeclass_instance.h"

#include "vapor/analyzer/expressions/expression_list.h"
#include "vapor/analyzer/semantic/symbol.h"
#include "vapor/analyzer/semantic/typeclass_instance.h"
#include "vapor/analyzer/statements/function.h"
#include "vapor/parser/expr.h"
#include "vapor/parser/typeclass.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    std::unique_ptr<typeclass_instance_expression> preanalyze_instance_literal(precontext & ctx,
        const parser::instance_literal & parse,
        scope * lex_scope)
    {
        auto late_preanalysis = [&parse, &ctx](function_definition_handler fn_def) {
            fmap(parse.definitions, [&](auto && definition) {
                fmap(definition, make_overload_set([&](const parser::function_definition & func) {
                    fn_def(ctx, func);
                    return unit{};
                }));
                return unit{};
            });
        };

        return std::make_unique<typeclass_instance_expression>(
            make_node(parse), std::move(late_preanalysis), make_typeclass_instance(ctx, parse, lex_scope));
    }

    typeclass_instance_expression::typeclass_instance_expression(ast_node parse,
        late_preanalysis_type late_pre,
        std::unique_ptr<typeclass_instance> instance)
        : _late_preanalysis{ std::move(late_pre) }, _instance{ std::move(instance) }
    {
        _set_ast_info(parse);
    }

    typeclass_instance_expression::~typeclass_instance_expression() = default;

    void typeclass_instance_expression::print(std::ostream & os, print_context ctx) const
    {
        assert(0);
    }

    future<> typeclass_instance_expression::_analyze(analysis_context & ctx)
    {
        auto name = _instance->typeclass_name();
        auto top_level = std::move(name.front());
        name.erase(name.begin());

        future<expression *> expr =
            _instance->get_scope()->parent()->resolve(top_level)->get_expression_future();

        if (!name.empty())
        {
            expr = foldl(name, std::move(expr), [&ctx](future<expression *> expr, auto && name) {
                return expr
                    .then([&](auto && expr) {
                        return expr->analyze(ctx).then([expr] { return expr->get_type()->get_scope(); });
                    })
                    .then([name = std::move(name)](
                        const scope * lex_scope) { return lex_scope->get(name)->get_expression_future(); });
            });
        }

        return expr.then([&](expression * expr) { return expr->analyze(ctx).then([expr] { return expr; }); })
            .then([&](expression * expr) {
                return when_all(
                    fmap(_instance->get_arguments(), [&](auto && arg) { return arg->analyze(ctx); }))
                    .then([&] { return _instance->simplify_arguments(ctx); })
                    .then([expr] { return expr; });
            })
            .then([&](expression * expr) {
                assert(0);

                /*auto tpl = expr->_get_replacement()->as<template_expression>();
                assert(tpl);
                auto instance_type_expr =
                    dynamic_cast<typeclass_literal_instance *>(ctx.get_instantiation(tpl, fmap(_arguments,
                [](auto && ptr) { return ptr.get(); }))); assert(instance_type_expr);
                _set_type(instance_type_expr->instance_type());

                _instance = make_typeclass_instance(instance_type_expr->instance_type());
                _late_preanalysis(_instance->get_function_definition_handler());

                return when_all(fmap(_instance->get_member_function_defs(), [&](auto && fn_def) { return
                fn_def->analyze(ctx); }));*/
            })
            .then([&] {
                assert(0);
                //_instance->import_default_definitions();
            });
    }

    std::unique_ptr<expression> typeclass_instance_expression::_clone_expr_with_replacement(
        replacements & repl) const
    {
        assert(0);
    }

    future<expression *> typeclass_instance_expression::_simplify_expr(recursive_context ctx)
    {
        assert(0);
    }

    statement_ir typeclass_instance_expression::_codegen_ir(ir_generation_context & ctx) const
    {
        assert(0);
    }
}
}
