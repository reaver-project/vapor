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

#include "vapor/analyzer/integer.h"
#include "vapor/codegen/ir/variable.h"

std::shared_ptr<reaver::vapor::codegen::_v1::ir::variable_type> reaver::vapor::analyzer::_v1::integer_type::_codegen_type() const
{
    return codegen::ir::builtin_types().integer;
}

reaver::vapor::analyzer::_v1::variable_ir reaver::vapor::analyzer::_v1::integer_constant::_codegen_ir() const
{
    return {
        codegen::ir::value{ codegen::ir::integer_value{ _value } }
    };
}

reaver::vapor::analyzer::_v1::statement_ir reaver::vapor::analyzer::_v1::integer_literal::_codegen_ir() const
{
    return { codegen::ir::instruction{
        none, none,
        { boost::typeindex::type_id<codegen::ir::pass_variable_instruction>() },
        {},
        codegen::ir::value{ codegen::ir::integer_value{ _value->get_value() } }
    } };
}

