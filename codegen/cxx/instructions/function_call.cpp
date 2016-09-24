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

template<>
std::u32string reaver::vapor::codegen::_v1::cxx::generate<reaver::vapor::codegen::_v1::ir::function_call_instruction>(const reaver::vapor::codegen::_v1::ir::instruction & inst, reaver::vapor::codegen::_v1::codegen_context & ctx)
{
    std::size_t actual_argument_offset = inst.operands.front().index() == 0 ? 2 : 1;

    std::u32string arguments;
    std::for_each(inst.operands.begin() + actual_argument_offset, inst.operands.end(), [&](auto && operand) {
        arguments += value_of(operand, ctx);
        arguments += U", ";
    });
    if (inst.operands.size() > actual_argument_offset)
    {
        arguments.pop_back();
        arguments.pop_back();
    }

    std::u32string base_variable;
    if (actual_argument_offset == 2) // member call
    {
        base_variable = variable_of(inst.operands.front(), ctx) + U".";
    }

    return variable_of(inst.result, ctx) + U" = " + base_variable + get<codegen::ir::label>(inst.operands[actual_argument_offset - 1]).name + U"(" + arguments + U");\n";
}

