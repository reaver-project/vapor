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

#include "vapor/codegen/ir/instruction.h"
#include "vapor/codegen/llvm_ir.h"

namespace reaver::vapor::codegen
{
inline namespace _v1
{
    namespace
    {
        template<typename... InstructionTypes>
        std::u32string generate_helper(const ir::instruction & inst, codegen_context & ctx)
        {
            using dispatched_type = std::u32string (*)(const ir::instruction &, codegen_context & ctx);

            auto generator = [](auto type_id) {
                using T = typename decltype(type_id)::type;
                return &llvm_ir_generator::generate<T>;
            };

            static const std::unordered_map<boost::typeindex::type_index, dispatched_type, boost::hash<boost::typeindex::type_index>> dispatch_table = {
                { boost::typeindex::type_id<InstructionTypes>(), generator(reaver::id<InstructionTypes>()) }...
            };

            return dispatch_table.at(inst.instruction.id())(inst, ctx);
        }
    }

    std::u32string llvm_ir_generator::generate(const ir::instruction & inst, codegen_context & ctx)
    {
        std::u32string base;
        if (inst.label)
        {
            base += *inst.label + U":\n";
        }

        if (inst.result.index() == 0)
        {
            auto && var = *get<std::shared_ptr<ir::variable>>(inst.result);
            if (!var.declared)
            {
                var.declared = true;
                llvm_ir_generator::variable_name(var, ctx);
            }
        }

        return base
            + generate_helper<ir::function_call_instruction,
                  ir::materialization_instruction,
                  ir::destruction_instruction,
                  ir::temporary_destruction_instruction,
                  ir::pass_value_instruction,
                  ir::return_instruction,
                  ir::jump_instruction,
                  ir::phi_instruction,
                  ir::noop_instruction,

                  ir::aggregate_init_instruction,
                  ir::member_access_instruction,

                  ir::integer_addition_instruction,
                  ir::integer_subtraction_instruction,
                  ir::integer_multiplication_instruction,
                  ir::integer_equal_comparison_instruction,
                  ir::integer_less_comparison_instruction,
                  ir::integer_less_equal_comparison_instruction,

                  ir::boolean_equal_comparison_instruction,
                  ir::boolean_negation_instruction>(inst, ctx);
    }

    template<>
    std::u32string llvm_ir_generator::generate<ir::pass_value_instruction>(const ir::instruction &, codegen_context &)
    {
        return {};
    }

    template<>
    std::u32string llvm_ir_generator::generate<ir::noop_instruction>(const ir::instruction &, codegen_context &)
    {
        return {};
    }
}
}
