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

#include "vapor/analyzer/expressions/struct.h"
#include "vapor/analyzer/statements/declaration.h"
#include "vapor/analyzer/symbol.h"
#include "vapor/parser/struct.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    std::unique_ptr<struct_literal> preanalyze_struct_literal(const parser::struct_literal & parse, scope * lex_scope)
    {
        return std::make_unique<struct_literal>(make_node(parse), make_struct_type(parse, lex_scope));
    }

    struct_literal::struct_literal(ast_node parse, std::unique_ptr<struct_type> type) : expression{ builtin_types().type.get() }, _type{ std::move(type) }
    {
        _set_ast_info(parse);
    }

    void struct_literal::print(std::ostream & os, print_context ctx) const
    {
        os << styles::def << ctx << styles::rule_name << "struct-literal";
        print_address_range(os, this);
        os << '\n';

        auto type_ctx = ctx.make_branch(_type->get_data_member_decls().empty());
        os << styles::def << type_ctx << styles::subrule_name << "defined type:\n";
        _type->print(os, type_ctx.make_branch(true));

        auto data_members_ctx = ctx.make_branch(true);
        os << styles::def << data_members_ctx << styles::subrule_name << "data member definitions:\n";

        std::size_t idx = 0;
        for (auto && member : _type->get_data_member_decls())
        {
            member->print(os, data_members_ctx.make_branch(++idx == _type->get_data_member_decls().size()));
        }
    }

    statement_ir struct_literal::_codegen_ir(ir_generation_context & ctx) const
    {
        return {};
    }
}
}
