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
#include "vapor/analyzer/symbol.h"
#include "vapor/analyzer/variables/member_assignment.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    void call_expression::print(std::ostream & os, std::size_t indent) const
    {
        if (_replacement_expr)
        {
            _replacement_expr->print(os, indent);
            return;
        }

        auto in = std::string(indent, ' ');
        os << in << "call expression at " << _range << '\n';
        os << in << "type: " << _var->get_type()->explain() << '\n';
        os << in << "function: " << _function->explain() << '\n';
        os << in << "arguments:\n";

        fmap(_args, [&](auto && arg) {
            os << in << "{\n";
            arg->print(os, indent + 4);
            os << in << "}\n";

            return unit{};
        });
    }

    variable * call_expression::get_variable() const
    {
        if (_replacement_expr)
        {
            return _replacement_expr->get_variable();
        }

        assert(_var);
        return _var.get();
    }

    statement_ir call_expression::_codegen_ir(ir_generation_context & ctx) const
    {
        if (_replacement_expr)
        {
            return _replacement_expr->codegen_ir(ctx);
        }

        auto arguments_instructions = fmap(_args, [&](auto && arg) { return arg->codegen_ir(ctx); });

        arguments_instructions.reserve(_function->parameters().size());
        std::transform(
            _function->parameters().begin() + _args.size(), _function->parameters().end(), std::back_inserter(arguments_instructions), [&](auto && member) {
                auto def = member->get_default_value();
                assert(def);
                return def->codegen_ir(ctx);
            });

        auto arguments_values = fmap(arguments_instructions, [](auto && insts) { return insts.back().result; });
        arguments_values.insert(arguments_values.begin(), _function->call_operand_ir(ctx));

        if (_function->is_member())
        {
            assert(arguments_values.size() >= 2);
            std::swap(arguments_values[0], arguments_values[1]);
        }

        auto call_expr_instruction = codegen::ir::instruction{ none,
            none,
            { boost::typeindex::type_id<codegen::ir::function_call_instruction>() },
            std::move(arguments_values),
            { codegen::ir::make_variable(_var->get_type()->codegen_type(ctx)) } };

        ctx.add_function_to_generate(_function);

        statement_ir ret;
        fmap(arguments_instructions, [&](auto && insts) {
            std::move(insts.begin(), insts.end(), std::back_inserter(ret));
            return unit{};
        });
        ret.push_back(std::move(call_expr_instruction));

        return ret;
    }
}
}
