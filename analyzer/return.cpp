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

#include "vapor/parser/expression_list.h"
#include "vapor/parser/lambda_expression.h"
#include "vapor/analyzer/return.h"
#include "vapor/analyzer/symbol.h"

void reaver::vapor::analyzer::_v1::return_statement::print(std::ostream & os, std::size_t indent) const
{
    auto in = std::string(indent, ' ');
    os << in << "return statement at " << _parse.range << '\n';
    os << in << "return value expression:\n";
    os << in << "{\n";
    _value_expr->print(os, indent + 4);
    os << in << "}\n";
}

reaver::vapor::analyzer::_v1::statement_ir reaver::vapor::analyzer::_v1::return_statement::_codegen_ir(reaver::vapor::analyzer::_v1::ir_generation_context & ctx) const
{
    auto ret = _value_expr->codegen_ir(ctx);
    ret.push_back({
        none, none,
        { boost::typeindex::type_id<codegen::ir::return_instruction>() },
        { ret.back().result },
        ret.back().result
    });

    return ret;
}

