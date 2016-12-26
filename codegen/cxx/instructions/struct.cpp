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

#include <boost/algorithm/string/join.hpp>

#include "vapor/codegen/cxx.h"
#include "vapor/codegen/ir/instruction.h"

namespace reaver::vapor::codegen
{
inline namespace _v1
{
    namespace cxx
    {
        template<>
        std::u32string generate<ir::aggregate_init_instruction>(const ir::instruction & inst, codegen_context & ctx)
        {
            std::u32string arguments;
            std::for_each(inst.operands.begin(), inst.operands.end(), [&](auto && operand) {
                arguments += value_of(operand, ctx);
                arguments += U", ";
            });
            if (inst.operands.size() > 0)
            {
                arguments.pop_back();
                arguments.pop_back();
            }

            return variable_of(inst.result, ctx) + U".emplace(" + arguments + U");\n";
        }

        template<>
        std::u32string generate<ir::member_access_instruction>(const ir::instruction & inst, codegen_context & ctx)
        {
            assert(inst.operands.size() == 2);
            return variable_of(inst.result, ctx) + U".emplace(" + variable_of(inst.operands[0], ctx) + U".reference()." + get<ir::label>(inst.operands[1]).name
                + U");\n";
        }
    }
}
}
