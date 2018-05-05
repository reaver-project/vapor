/**
 * Vapor Compiler Licence
 *
 * Copyright © 2017-2018 Michał "Griwes" Dominiak
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
#include "vapor/codegen/ir/type.h"
#include "vapor/codegen/llvm_ir.h"

namespace reaver::vapor::codegen
{
inline namespace _v1
{
    template<>
    std::u32string llvm_ir_generator::generate<ir::function_call_instruction>(const ir::instruction & inst, codegen_context & ctx)
    {
        std::size_t actual_argument_offset = inst.operands.front().index() == 0 ? 2 : 1;

        std::u32string arguments;
        std::for_each(inst.operands.begin() + actual_argument_offset, inst.operands.end(), [&](auto && operand) {
            fmap(operand,
                make_overload_set(
                    [&](std::shared_ptr<ir::variable> var) {
                        ctx.put_into_global_before += ctx.define_if_necessary(var->type);
                        return unit{};
                    },
                    [&](auto &&) { return unit{}; }));

            arguments += type_of(operand, ctx);
            arguments += U" ";
            arguments += value_of(operand, ctx);
            arguments += U", ";
        });
        if (inst.operands.size() > actual_argument_offset)
        {
            arguments.pop_back();
            arguments.pop_back();
        }

        auto call_operand = std::get<codegen::ir::label>(inst.operands[actual_argument_offset - 1]);
        std::u32string call_operand_str;

        for (auto && scope : call_operand.scopes)
        {
            call_operand_str += scope.name + U".";
        }
        call_operand_str += call_operand.name;

        return variable_of(inst.result, ctx) + U" = call " + type_of(inst.result, ctx) + U" @\"" + call_operand_str + U"\"(" + arguments + U")\n";
    }
}
}
