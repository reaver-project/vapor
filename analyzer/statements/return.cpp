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

#include "vapor/analyzer/statements/return.h"
#include "vapor/analyzer/symbol.h"
#include "vapor/parser/expression_list.h"
#include "vapor/parser/lambda_expression.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    void return_statement::print(std::ostream & os, print_context ctx) const
    {
        os << styles::def << ctx << styles::rule_name << "return-statement";
        print_address_range(os, this);
        os << '\n';

        auto value_ctx = ctx.make_branch(true);
        os << styles::def << value_ctx << styles::subrule_name << "return value expression:\n";
        _value_expr->print(os, value_ctx.make_branch(true));
    }

    statement_ir return_statement::_codegen_ir(ir_generation_context & ctx) const
    {
        auto ret = _value_expr->codegen_ir(ctx);
        ret.push_back({ none, none, { boost::typeindex::type_id<codegen::ir::return_instruction>() }, { ret.back().result }, ret.back().result });

        return ret;
    }
}
}
