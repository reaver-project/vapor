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

#include "vapor/codegen/ir/instruction.h"
#include "vapor/codegen/llvm_ir.h"

namespace reaver::vapor::codegen
{
inline namespace _v1
{
    // TODO: this implementation is BAD
    // I need to align the branching model I have to what LLVM has
    // because otherwise I'll forever have to deal with this insanity
    template<>
    std::u32string llvm_ir_generator::generate<ir::jump_instruction>(const ir::instruction & inst, codegen_context & ctx)
    {
        assert(inst.operands.size() > 0 && inst.operands.size() % 2 == 0);

        auto br_label_id = ctx.unnamed_variable_index++;

        std::u32string ret;
        for (std::size_t i = 0; i < inst.operands.size() / 2; ++i)
        {
            auto label = utf32("." + std::to_string(br_label_id) + "." + std::to_string(i));

            ret += U"br i1 " + value_of(inst.operands[i * 2], ctx) + U", label " + value_of(inst.operands[i * 2 + 1], ctx) + U", label %\"" + label + U"\"\n"
                + label + U":\n";
        }

        return ret;
    }
}
}
