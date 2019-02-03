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
        auto name_id_expr =
            fmap(parse.typeclass_name.id_expression_value, [&](auto && token) { return token.value.string; });

        auto late_preanalysis = [&parse, &ctx](function_definition_handler fn_def) {
            fmap(parse.definitions, [&](auto && definition) {
                fmap(definition, make_overload_set([&](const parser::function_definition & func) {
                    fn_def(ctx, func);
                    return unit{};
                }));
                return unit{};
            });
        };

        return std::make_unique<typeclass_instance_expression>(make_node(parse),
            lex_scope,
            std::move(name_id_expr),
            fmap(parse.arguments.expressions,
                [&](auto && expr) { return preanalyze_expression(ctx, expr, lex_scope); }),
            std::move(late_preanalysis));
    }

    typeclass_instance_expression::typeclass_instance_expression(ast_node parse,
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

    typeclass_instance_expression::~typeclass_instance_expression() = default;

    void typeclass_instance_expression::print(std::ostream & os, print_context ctx) const
    {
        assert(0);
    }

    future<> typeclass_instance_expression::_analyze(analysis_context & ctx)
    {
        auto name = _typeclass_name;
        auto top_level = std::move(name.front());
        name.erase(name.begin());

        future<expression *> expr = _original_scope->resolve(top_level)->get_expression_future();

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

        // TODO: this can be refactored to use the typeclass template's call operator
        // I don't think that is *really* necessary, but might lead to a very slight cleanup here
        return expr.then([&](expression * expr) { return expr->analyze(ctx).then([expr] { return expr; }); })
            .then([&](expression * expr) {
                return when_all(fmap(_arguments,
                                    [&](auto && arg) {
                                        return arg->analyze(ctx).then(
                                            [&] { return simplification_loop(ctx, arg); });
                                    }))
                    .then([expr](auto &&) { return expr; });
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
