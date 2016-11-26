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

#include "vapor/analyzer/expressions/boolean.h"
#include "vapor/analyzer/symbol.h"
#include "vapor/codegen/ir/type.h"
#include "vapor/codegen/ir/variable.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    statement_ir boolean_literal::_codegen_ir(ir_generation_context &) const
    {
        return { codegen::ir::instruction{ none,
            none,
            { boost::typeindex::type_id<codegen::ir::pass_value_instruction>() },
            {},
            codegen::ir::value{ codegen::ir::boolean_value{ _value->get_value() } } } };
    }
}
}
