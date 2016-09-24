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
std::u32string reaver::vapor::codegen::_v1::cxx::generate<reaver::vapor::codegen::_v1::ir::declaration_instruction>(const reaver::vapor::codegen::_v1::ir::instruction & inst, reaver::vapor::codegen::_v1::codegen_context & ctx)
{
    assert(inst.declared_variable);
    assert(inst.operands.empty());
    auto && var = **inst.declared_variable;
    return type_name(var.type, ctx) + U" " + declaration_variable_name(var, ctx) + U";\n";
}

