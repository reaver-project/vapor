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

#include "vapor/codegen/cxx.h"
#include "vapor/codegen/ir/instruction.h"
#include "vapor/codegen/cxx/names.h"

namespace reaver::vapor::codegen { inline namespace _v1
{
    namespace cxx
    {
#define ADD_INSTRUCTION(NAME, OPERATOR) \
        template<> \
        std::u32string generate<ir::integer_ ## NAME ## _instruction>(const ir::instruction & inst, codegen_context & ctx) \
        { \
            assert(inst.operands.size() == 2); \
            return variable_of(inst.result, ctx) + U".emplace(" + value_of(inst.operands[0], ctx) + OPERATOR + value_of(inst.operands[1], ctx) + U");\n"; \
        }

        ADD_INSTRUCTION(addition, U" + ");
        ADD_INSTRUCTION(subtraction, U" - ");
        ADD_INSTRUCTION(multiplication, U" * ");
        ADD_INSTRUCTION(equal_comparison, U" == ");
        ADD_INSTRUCTION(less_comparison, U" < ");
        ADD_INSTRUCTION(less_equal_comparison, U" <= ");
    }
}}

