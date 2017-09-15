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

#include "vapor/codegen/generator.h"
#include "vapor/codegen/ir/instruction.h"
#include "vapor/codegen/llvm_ir.h"

namespace reaver::vapor::codegen
{
inline namespace _v1
{
    template<>
    std::u32string llvm_ir_generator::generate<ir::phi_instruction>(const ir::instruction & inst, codegen_context & ctx)
    {
        std::u32string ret;

        ret += variable_of(inst.result, ctx) + U" = phi " + type_of(inst.result, ctx);

        for (std::size_t i = 0; 2 * i < inst.operands.size(); ++i)
        {
            ret += U" [ " + value_of(inst.operands[i * 2 + 1], ctx) + U", " + value_of(inst.operands[i * 2], ctx) + U" ],";
        }
        assert(ret.back() == U',');
        ret.back() = U'\n';

        return ret;
    }
}
}
