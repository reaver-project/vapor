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
#include "vapor/parser/member_expression.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    std::unique_ptr<member_access_expression> preanalyze_member_access_expression(const parser::member_expression & parse, scope *)
    {
        return std::make_unique<member_access_expression>(make_node(parse), parse.member_name.value.string);
    }

    void member_access_expression::print(std::ostream & os, print_context ctx) const
    {
        os << styles::def << ctx << styles::rule_name << "member-access-expression";
        if (get_ast_info())
        {
            print_address_range(os, this);
        }
        else
        {
            os << styles::def << " @ " << styles::address << this << styles::def << ": ";
        }
        os << styles::string_value << ' ' << utf8(_name) << '\n';

        auto type_ctx = ctx.make_branch(true);
        os << styles::def << type_ctx << styles::subrule_name << "referenced member type:\n";
        get_type()->print(os, type_ctx.make_branch(true));
    }

    statement_ir member_access_expression::_codegen_ir(ir_generation_context & ctx) const
    {
        _base = _base ? _base : ctx.get_current_base();

        auto base_variable_value = _base->codegen_ir(ctx).back().result;
        auto base_variable = get<std::shared_ptr<codegen::ir::variable>>(base_variable_value);

        auto retvar = codegen::ir::make_variable(_base->get_type()->get_member_type(_name)->codegen_type(ctx));

        return { codegen::ir::instruction{
            none, none, { boost::typeindex::type_id<codegen::ir::member_access_instruction>() }, { base_variable, codegen::ir::label{ _name, {} } }, retvar } };
    }

    bool member_access_expression::_invalidate_ir(ir_generation_context & ctx) const
    {
        return ctx.get_current_base() != _base;
    }
}
}
