/**
 * Vapor Compiler Licence
 *
 * Copyright © 2016 Michał "Griwes" Dominiak
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

#include "vapor/analyzer/integer.h"
#include "vapor/codegen/ir/variable.h"
#include "vapor/codegen/ir/type.h"

std::shared_ptr<reaver::vapor::codegen::_v1::ir::variable_type> reaver::vapor::analyzer::_v1::integer_type::_codegen_type(reaver::vapor::analyzer::_v1::ir_generation_context &) const
{
    return codegen::ir::builtin_types().integer;
}

reaver::vapor::analyzer::_v1::variable_ir reaver::vapor::analyzer::_v1::integer_constant::_codegen_ir(reaver::vapor::analyzer::_v1::ir_generation_context &) const
{
    return {
        codegen::ir::value{ codegen::ir::integer_value{ _value } }
    };
}

reaver::vapor::analyzer::_v1::statement_ir reaver::vapor::analyzer::_v1::integer_literal::_codegen_ir(reaver::vapor::analyzer::_v1::ir_generation_context &) const
{
    return { codegen::ir::instruction{
        none, none,
        { boost::typeindex::type_id<codegen::ir::pass_value_instruction>() },
        {},
        codegen::ir::value{ codegen::ir::integer_value{ _value->get_value() } }
    } };
}

template<typename Instruction, typename Eval>
auto reaver::vapor::analyzer::_v1::integer_type::_generate_function(const char32_t * name, Eval eval)
{
    auto fun = make_function(
        "<builtin integer addition>",
        builtin_types().integer,
        { builtin_types().integer, builtin_types().integer },
        [name](ir_generation_context & ctx) {
            auto lhs = codegen::ir::make_variable(
                builtin_types().integer->codegen_type(ctx)
            );
            auto rhs = codegen::ir::make_variable(
                builtin_types().integer->codegen_type(ctx)
            );

            auto retval = codegen::ir::make_variable(
                builtin_types().integer->codegen_type(ctx)
            );

            return codegen::ir::function{
                name,
                {}, { lhs, rhs },
                retval,
                {
                    codegen::ir::instruction{
                        none, none,
                        { boost::typeindex::type_id<Instruction>() },
                        { lhs, rhs },
                        retval
                    },
                    codegen::ir::instruction{
                        none, none,
                        { boost::typeindex::type_id<codegen::ir::return_instruction>() },
                        {},
                        retval
                    }
                }
            };
        }
    );
    fun->set_eval(eval);
    return fun;
}

std::shared_ptr<reaver::vapor::analyzer::_v1::function> reaver::vapor::analyzer::_v1::integer_type::_addition()
{
    static auto eval = [](auto &&, const std::vector<std::shared_ptr<variable>> & args) {
        assert(args.size() == 2);
        assert(args[0]->get_type() == builtin_types().integer);
        assert(args[1]->get_type() == builtin_types().integer);

        if (!args[0]->is_constant() || !args[1]->is_constant())
        {
            return std::shared_ptr<expression>();
        }

        auto lhs = std::static_pointer_cast<integer_constant>(args[0]);
        auto rhs = std::static_pointer_cast<integer_constant>(args[1]);
        return make_variable_expression(std::make_shared<integer_constant>(lhs->get_value() + rhs->get_value()));

    };
    static auto addition = _generate_function<codegen::ir::integer_addition_instruction>(U"__builtin_integer_operator_plus", eval);
    return addition;
}

std::shared_ptr<reaver::vapor::analyzer::_v1::function> reaver::vapor::analyzer::_v1::integer_type::_multiplication()
{
    static auto eval = [](auto &&, const std::vector<std::shared_ptr<variable>> & args) {
        assert(args.size() == 2);
        assert(args[0]->get_type() == builtin_types().integer);
        assert(args[1]->get_type() == builtin_types().integer);

        if (!args[0]->is_constant() || !args[1]->is_constant())
        {
            return std::shared_ptr<expression>();
        }

        auto lhs = std::static_pointer_cast<integer_constant>(args[0]);
        auto rhs = std::static_pointer_cast<integer_constant>(args[1]);
        return make_variable_expression(std::make_shared<integer_constant>(lhs->get_value() * rhs->get_value()));

    };
    static auto multiplication = _generate_function<codegen::ir::integer_multiplication_instruction>(U"__builtin_integer_operator_star", eval);
    return multiplication;
}

