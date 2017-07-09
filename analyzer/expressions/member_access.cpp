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

#include "vapor/analyzer/expressions/member_access.h"
#include "vapor/analyzer/symbol.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    member_access_expression::member_access_expression(const parser::member_expression & parse) : _parse{ parse }
    {
    }

    void member_access_expression::print(std::ostream & os, print_context ctx) const
    {
        os << styles::def << ctx << styles::rule_name << "member-access-expression";
        print_address_range(os, this);
        os << '\n';

        os << styles::def << ctx.make_branch(false) << styles::subrule_name << "referenced member name: " << styles::string_value
           << utf8(_parse.member_name.value.string) << '\n';

        auto type_ctx = ctx.make_branch(true);
        os << styles::def << type_ctx << styles::subrule_name << "referenced member type:\n";
        get_type()->print(os, type_ctx.make_branch(true));
    }

    statement_ir member_access_expression::_codegen_ir(ir_generation_context & ctx) const
    {
        auto base_variable_value = _base->codegen_ir(ctx).back().result;
        auto base_variable = get<std::shared_ptr<codegen::ir::variable>>(base_variable_value);

        return { codegen::ir::instruction{ none,
                     none,
                     { boost::typeindex::type_id<codegen::ir::member_access_instruction>() },
                     { base_variable, codegen::ir::label{ _parse.member_name.value.string, {} } },
                     _referenced->codegen_ir(ctx).back().result },
            codegen::ir::instruction{
                none, none, { boost::typeindex::type_id<codegen::ir::pass_value_instruction>() }, {}, _referenced->codegen_ir(ctx).back().result } };
    }
}
}
