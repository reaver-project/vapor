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

#include <unordered_map>

#include <boost/type_index.hpp>

#include <reaver/id.h>

#include "vapor/codegen/cxx.h"
#include "vapor/codegen/cxx/names.h"
#include "vapor/codegen/ir/instruction.h"

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
                return &cxx::generate<T>;
            };

            static const std::unordered_map<boost::typeindex::type_index, dispatched_type, boost::hash<boost::typeindex::type_index>> dispatch_table = {
                { boost::typeindex::type_id<InstructionTypes>(), generator(reaver::id<InstructionTypes>()) }...
            };

            return dispatch_table.at(inst.instruction.id())(inst, ctx);
        }
    }

    std::u32string cxx_generator::generate(const ir::instruction & inst, codegen_context & ctx) const
    {
        std::u32string base;
        if (inst.label)
        {
            base += *inst.label + U":\n";
        }

        // make all pieces of storage that are moved-from during this instruction
        // available for storage of the result of this instruction
        fmap(inst.operands, [&](auto && operand) {
            return fmap(operand,
                make_overload_set(
                    [&](const std::shared_ptr<ir::variable> & var) {
                        if (!var->argument && var->is_move())
                        {
                            cxx::mark_destroyed(var, ctx);
                        }
                        return unit{};
                    },
                    [&](auto &&) { return unit{}; }));
        });

        if (inst.result.index() == 0)
        {
            auto && var = *get<std::shared_ptr<ir::variable>>(inst.result);
            if (!var.declared)
            {
                var.declared = true;
                var.name = ctx.get_storage_for(var.type);
            }
        }

        return base + generate_helper<ir::function_call_instruction,
                          ir::materialization_instruction,
                          ir::destruction_instruction,
                          ir::temporary_destruction_instruction,
                          ir::pass_value_instruction,
                          ir::return_instruction,
                          ir::jump_instruction,
                          ir::phi_instruction,
                          ir::noop_instruction,

                          ir::integer_addition_instruction,
                          ir::integer_subtraction_instruction,
                          ir::integer_multiplication_instruction,
                          ir::integer_equal_comparison_instruction,
                          ir::integer_less_comparison_instruction,
                          ir::integer_less_equal_comparison_instruction,

                          ir::boolean_equal_comparison_instruction,
                          ir::boolean_negation_instruction>(inst, ctx);
    }

    namespace cxx
    {
        template<>
        std::u32string generate<ir::pass_value_instruction>(const ir::instruction &, codegen_context &)
        {
            return {};
        }
    }

    namespace cxx
    {
        template<>
        std::u32string generate<ir::noop_instruction>(const ir::instruction &, codegen_context &)
        {
            return {};
        }
    }
}
}
