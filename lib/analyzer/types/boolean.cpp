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

#include "vapor/analyzer/types/boolean.h"
#include "vapor/analyzer/expressions/boolean.h"
#include "vapor/analyzer/expressions/runtime_value.h"
#include "vapor/analyzer/symbol.h"
#include "vapor/codegen/ir/instruction.h"
#include "vapor/codegen/ir/type.h"
#include "vapor/codegen/ir/variable.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    void boolean_type::_codegen_type(ir_generation_context &) const
    {
        _codegen_t = codegen::ir::builtin_types().boolean;
    }

    std::unique_ptr<type> make_boolean_type()
    {
        return std::make_unique<boolean_type>();
    }

    template<typename Instruction, typename Eval>
    auto boolean_type::_generate_function(const char32_t * name, const char * desc, Eval eval, type * return_type)
    {
        auto lhs = make_runtime_value(builtin_types().boolean.get());
        auto rhs = make_runtime_value(builtin_types().boolean.get());

        auto lhs_arg = lhs.get();
        auto rhs_arg = rhs.get();

        auto fun = make_function(desc, return_type->get_expression(), { lhs_arg, rhs_arg }, [name,
            return_type,
            lhs = std::move(lhs),
            rhs = std::move(rhs)](ir_generation_context & ctx) {
            auto lhs_ir = get_ir_variable(lhs->codegen_ir(ctx));
            auto rhs_ir = get_ir_variable(rhs->codegen_ir(ctx));

            auto retval = codegen::ir::make_variable(return_type->codegen_type(ctx));

            return codegen::ir::function{ name,
                {},
                { lhs_ir, rhs_ir },
                retval,
                { codegen::ir::instruction{ std::nullopt, std::nullopt, { boost::typeindex::type_id<Instruction>() }, { lhs_ir, rhs_ir }, retval },
                    codegen::ir::instruction{ std::nullopt, std::nullopt, { boost::typeindex::type_id<codegen::ir::return_instruction>() }, {}, retval } } };
        });
        fun->set_name(name);
        fun->set_eval(eval);
        return fun;
    }

#define ADD_OPERATION(NAME, BUILTIN_NAME, OPERATOR, RESULT_TYPE)                                                                                               \
    function * boolean_type::_##NAME()                                                                                                                         \
    {                                                                                                                                                          \
        static auto eval = [](auto &&, const std::vector<expression *> & args) {                                                                               \
            assert(args.size() == 2);                                                                                                                          \
            assert(args[0]->get_type() == builtin_types().boolean.get());                                                                                      \
            assert(args[1]->get_type() == builtin_types().boolean.get());                                                                                      \
                                                                                                                                                               \
            if (!args[0]->is_constant() || !args[1]->is_constant())                                                                                            \
            {                                                                                                                                                  \
                return make_ready_future<expression *>(nullptr);                                                                                               \
            }                                                                                                                                                  \
                                                                                                                                                               \
            auto lhs = args[0]->as<boolean_constant>();                                                                                                        \
            auto rhs = args[1]->as<boolean_constant>();                                                                                                        \
            return make_ready_future<expression *>(std::make_unique<RESULT_TYPE##_constant>(lhs->get_value() OPERATOR rhs->get_value()).release());            \
        };                                                                                                                                                     \
        static auto NAME = _generate_function<codegen::ir::boolean_##NAME##_instruction>(                                                                      \
            BUILTIN_NAME, "<builtin boolean " #NAME ">", eval, builtin_types().RESULT_TYPE.get());                                                             \
        return NAME.get();                                                                                                                                     \
    }

    ADD_OPERATION(equal_comparison, U"__builtin_boolean_operator_equals", ==, boolean);
}
}
