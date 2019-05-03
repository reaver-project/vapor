/**
 * Vapor Compiler Licence
 *
 * Copyright © 2017, 2019 Michał "Griwes" Dominiak
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
#define ADD_INSTRUCTION(NAME, INSTRUCTION)                                                                   \
    template<>                                                                                               \
    std::u32string llvm_ir_generator::generate<ir::integer_##NAME##_instruction>(                            \
        const ir::instruction & inst, codegen_context & ctx)                                                 \
    {                                                                                                        \
        assert(inst.operands.size() == 2);                                                                   \
        return variable_of(inst.result, ctx) + U" = " + INSTRUCTION + U" " + type_of(inst.operands[0], ctx)  \
            + U" " + value_of(inst.operands[0], ctx) + U", " + value_of(inst.operands[1], ctx) + U"\n";      \
    }

    ADD_INSTRUCTION(addition, U"add");
    ADD_INSTRUCTION(subtraction, U"sub");
    ADD_INSTRUCTION(multiplication, U"mul");
    ADD_INSTRUCTION(division, U"sdiv"); // NOTE to future self: handle sdiv/udiv when introducing unsigned
    ADD_INSTRUCTION(equal_comparison, U"icmp eq");
    ADD_INSTRUCTION(less_comparison, U"icmp slt");
    ADD_INSTRUCTION(less_equal_comparison, U"icmp sle");
}
}
