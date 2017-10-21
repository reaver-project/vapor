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

#include "vapor/analyzer/expressions/closure.h"
#include "vapor/analyzer/helpers.h"
#include "vapor/analyzer/symbol.h"
#include "vapor/analyzer/types/closure.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    closure::closure(const parser::lambda_expression & parse, scope * lex_scope) : _scope{ lex_scope->clone_local() }
    {
        _set_ast_info(make_node(parse));

        assert(!parse.captures);

        fmap(parse.parameters, [&](auto && param_list) {
            _parameter_list = preanalyze_parameter_list(param_list, _scope.get());
            return unit{};
        });
        _scope->close();

        _return_type = fmap(parse.return_type, [&](auto && ret_type) { return preanalyze_expression(ret_type, _scope.get()); });
        _body = preanalyze_block(parse.body, _scope.get(), true);
    }

    void closure::print(std::ostream & os, print_context ctx) const
    {
        os << styles::def << ctx << styles::rule_name << "closure";
        print_address_range(os, this);
        os << '\n';

        auto type_ctx = ctx.make_branch(false);
        os << styles::def << type_ctx << styles::subrule_name << "type:\n";
        get_type()->print(os, type_ctx.make_branch(true));

        if (_parameter_list.size())
        {
            auto param_ctx = ctx.make_branch(false);
            os << styles::def << param_ctx << styles::subrule_name << "parameters:\n";

            std::size_t idx = 0;
            for (auto && param : _parameter_list)
            {
                param->print(os, param_ctx.make_branch(++idx == _parameter_list.size()));
            }
        }

        auto return_type_ctx = ctx.make_branch(false);
        os << styles::def << return_type_ctx << styles::subrule_name << "return type:\n";
        _body->return_type()->print(os, return_type_ctx.make_branch(true));

        auto body_ctx = ctx.make_branch(true);
        os << styles::def << body_ctx << styles::subrule_name << "body:\n";
        _body->print(os, body_ctx.make_branch(true));
    }

    statement_ir closure::_codegen_ir(ir_generation_context & ctx) const
    {
        auto var = codegen::ir::make_variable(get_type()->codegen_type(ctx));
        var->scopes = _type->get_scope()->codegen_ir(ctx);
        return { codegen::ir::instruction{ none, none, { boost::typeindex::type_id<codegen::ir::materialization_instruction>() }, {}, { std::move(var) } } };
    }

    declaration_ir closure::declaration_codegen_ir(ir_generation_context & ctx) const
    {
        return { { get<std::shared_ptr<codegen::ir::variable>>(codegen_ir(ctx).back().result) } };
    }
}
}
