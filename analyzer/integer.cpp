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
#include "vapor/analyzer/boolean.h"
#include "vapor/analyzer/symbol.h"
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

std::unique_ptr<reaver::vapor::analyzer::_v1::type> reaver::vapor::analyzer::_v1::make_integer_type()
{
    return std::make_unique<integer_type>();
}

template<typename Instruction, typename Eval>
auto reaver::vapor::analyzer::_v1::integer_type::_generate_function(const char32_t * name, const char * desc, Eval eval, reaver::vapor::analyzer::_v1::type * return_type)
{
    auto lhs = make_blank_variable(builtin_types().integer.get());
    auto rhs = make_blank_variable(builtin_types().integer.get());

    auto lhs_arg = lhs.get();
    auto rhs_arg = rhs.get();

    auto fun = make_function(
        desc,
        return_type,
        { lhs_arg, rhs_arg },
        [name, return_type, lhs = std::move(lhs), rhs = std::move(rhs)](ir_generation_context & ctx) {
            auto lhs_ir = get_ir_variable(lhs->codegen_ir(ctx));
            auto rhs_ir = get_ir_variable(rhs->codegen_ir(ctx));

            auto retval = codegen::ir::make_variable(
                return_type->codegen_type(ctx)
            );

            return codegen::ir::function{
                name,
                {}, { lhs_ir, rhs_ir },
                retval,
                {
                    codegen::ir::instruction{
                        none, none,
                        { boost::typeindex::type_id<Instruction>() },
                        { lhs_ir, rhs_ir },
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

#define ADD_OPERATION(NAME, BUILTIN_NAME, OPERATOR, RESULT_TYPE) \
    reaver::vapor::analyzer::_v1::function * reaver::vapor::analyzer::_v1::integer_type::_ ## NAME() \
    { \
        static auto eval = [](auto &&, const std::vector<variable *> & args) { \
            assert(args.size() == 2); \
            assert(args[0]->get_type() == builtin_types().integer.get()); \
            assert(args[1]->get_type() == builtin_types().integer.get()); \
 \
            if (!args[0]->is_constant() || !args[1]->is_constant()) \
            { \
                return (expression *)nullptr; \
            } \
 \
            auto lhs = static_cast<integer_constant *>(args[0]); \
            auto rhs = static_cast<integer_constant *>(args[1]); \
            return make_variable_expression(std::make_unique<RESULT_TYPE ## _constant>(lhs->get_value() OPERATOR rhs->get_value())).release(); \
        }; \
        static auto NAME = _generate_function<codegen::ir::integer_ ## NAME ## _instruction>( \
            BUILTIN_NAME, "<builtin integer " #NAME ">", \
            eval, builtin_types(). RESULT_TYPE .get()); \
        return NAME.get(); \
    }

ADD_OPERATION(addition, U"__builtin_integer_operator_plus", +, integer);
ADD_OPERATION(subtraction, U"__builtin_integer_operator_minus", -, integer);
ADD_OPERATION(multiplication, U"__builtin_integer_operator_star", *, integer);
ADD_OPERATION(equal_comparison, U"__builtin_integer_operator_equals", ==, boolean);
ADD_OPERATION(less_comparison, U"__builtin_integer_operator_less", <, boolean);

