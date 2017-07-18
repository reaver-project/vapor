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

#include "vapor/analyzer/expressions/call.h"
#include "vapor/analyzer/expressions/member_assignment.h"
#include "vapor/analyzer/symbol.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    void call_expression::print(std::ostream & os, print_context ctx) const
    {
        os << styles::def << ctx << styles::rule_name << "call-expression";
        print_address_range(os, this);
        os << '\n';

        if (_replacement_expr)
        {
            auto replacement_ctx = ctx.make_branch(true);

            os << styles::def << replacement_ctx << styles::subrule_name << "replacement expression:\n";
            _replacement_expr->print(os, replacement_ctx.make_branch(true));
            return;
        }

        auto type_ctx = ctx.make_branch(false);
        os << styles::def << type_ctx << styles::subrule_name << "type:\n";
        get_type()->print(os, type_ctx.make_branch(true));

        auto function_ctx = ctx.make_branch(_args.empty());
        os << styles::def << function_ctx << styles::subrule_name << "function:\n";
        _function->print(os, function_ctx.make_branch(true));

        if (_args.size())
        {
            auto args_ctx = ctx.make_branch(true);
            os << styles::def << args_ctx << styles::subrule_name << "arguments:\n";

            std::size_t idx = 0;
            for (auto && arg : _args)
            {
                arg->print(os, args_ctx.make_branch(++idx == _args.size()));
            }
        }
    }

    statement_ir call_expression::_codegen_ir(ir_generation_context & ctx) const
    {
        if (_replacement_expr)
        {
            return _replacement_expr->codegen_ir(ctx);
        }

        if (_function->is_member())
        {
            ctx.push_base_expression(_args.front());
        }

        auto arguments_instructions = fmap(_args, [&](auto && arg) { return arg->codegen_ir(ctx); });

        auto arguments_values = fmap(arguments_instructions, [](auto && insts) { return insts.back().result; });
        arguments_values.insert(arguments_values.begin(), _function->call_operand_ir(ctx));

        if (_function->is_member())
        {
            assert(arguments_values.size() >= 2);
            std::swap(arguments_values[0], arguments_values[1]);
            auto base = arguments_values[0];
        }

        auto call_expr_instruction = codegen::ir::instruction{ none,
            none,
            { boost::typeindex::type_id<codegen::ir::function_call_instruction>() },
            std::move(arguments_values),
            { codegen::ir::make_variable(get_type()->codegen_type(ctx)) } };

        ctx.add_function_to_generate(_function);

        statement_ir ret;
        fmap(arguments_instructions, [&](auto && insts) {
            std::move(insts.begin(), insts.end(), std::back_inserter(ret));
            return unit{};
        });
        ret.push_back(std::move(call_expr_instruction));

        if (_function->is_member())
        {
            ctx.pop_base_expression(_args.front());
        }

        return ret;
    }
}
}
