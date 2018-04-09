/**
 * Vapor Compiler Licence
 *
 * Copyright © 2016-2018 Michał "Griwes" Dominiak
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

#include <numeric>

#include "vapor/analyzer/expressions/overload_set.h"
#include "vapor/analyzer/function.h"
#include "vapor/analyzer/helpers.h"
#include "vapor/analyzer/statements/block.h"
#include "vapor/analyzer/statements/function.h"
#include "vapor/parser/expr.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    std::unique_ptr<function_definition> preanalyze_function_definition(precontext & ctx, const parser::function_definition & parse, scope *& lex_scope)
    {
        auto function_scope = lex_scope->clone_local();

        parameter_list params;
        if (parse.signature.parameters)
        {
            params = preanalyze_parameter_list(ctx, parse.signature.parameters.value(), function_scope.get());
        }
        function_scope->close();

        auto ret = std::make_unique<function_definition>(make_node(parse),
            parse.signature.name.value.string,
            std::move(params),
            fmap(parse.signature.return_type, [&](auto && ret_type) { return preanalyze_expression(ctx, ret_type, function_scope.get()); }),
            preanalyze_block(ctx, *parse.body, function_scope.get(), true),
            std::move(function_scope));

        if (parse.signature.export_)
        {
            lex_scope->get(parse.signature.name.value.string)->mark_exported();
            lex_scope->get(U"overload_set_type$" + parse.signature.name.value.string)->mark_exported();
        }

        return ret;
    }

    function_definition::function_definition(ast_node parse,
        std::u32string name,
        parameter_list params,
        std::optional<std::unique_ptr<expression>> return_type,
        std::unique_ptr<block> body,
        std::unique_ptr<scope> scope)
        : _name{ std::move(name) },
          _parameter_list{ std::move(params) },
          _return_type{ std::move(return_type) },
          _body{ std::move(body) },
          _scope{ std::move(scope) }
    {
        _set_ast_info(parse);

        auto type_name = U"overload_set_type$" + _name;

        std::shared_ptr<overload_set> keep_count;
        auto symbol = _scope->parent()->try_get(_name);

        if (!symbol)
        {
            keep_count = std::make_shared<overload_set>(_scope.get());
            symbol = _scope->parent()->init(_name, make_symbol(_name, keep_count.get()));

            auto type = keep_count->get_type();
            type->set_name(type_name);
            _scope->parent()->init(type_name, make_symbol(type_name, type->get_expression()));
        }

        _overload_set = symbol.value()->get_expression()->as<overload_set>()->shared_from_this();
    }

    void function_definition::print(std::ostream & os, print_context ctx) const
    {
        os << styles::def << ctx << styles::rule_name << "function-definition";
        print_address_range(os, this);
        os << ' ' << styles::string_value << utf8(_name) << '\n';

        if (_parameter_list.size())
        {
            auto params_ctx = ctx.make_branch(false);
            os << styles::def << params_ctx << styles::subrule_name << "parameters:\n";

            std::size_t idx = 0;
            for (auto && param : _parameter_list)
            {
                param->print(os, params_ctx.make_branch(++idx == _parameter_list.size()));
            }
        }

        auto return_type_ctx = ctx.make_branch(false);
        os << styles::def << return_type_ctx << styles::subrule_name << "return type:\n";
        _function->return_type_expression()->print(os, return_type_ctx.make_branch(true));

        auto body_ctx = ctx.make_branch(true);
        os << styles::def << body_ctx << styles::subrule_name << "body:\n";
        _body->print(os, body_ctx.make_branch(true));
    }

    statement_ir function_definition::_codegen_ir(ir_generation_context &) const
    {
        return {};
    }
}
}
