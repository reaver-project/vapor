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

#include <boost/type_index.hpp>

#include "vapor/analyzer/expressions/binary.h"
#include "vapor/analyzer/function.h"
#include "vapor/analyzer/helpers.h"
#include "vapor/analyzer/symbol.h"
#include "vapor/parser.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    std::unique_ptr<binary_expression> preanalyze_binary_expression(const parser::binary_expression & parse, scope * lex_scope)
    {
        return std::make_unique<binary_expression>(
            make_node(parse), parse.op, preanalyze_expression(parse.lhs, lex_scope), preanalyze_expression(parse.rhs, lex_scope));
    }

    binary_expression::binary_expression(ast_node parse, lexer::token op, std::unique_ptr<expression> lhs, std::unique_ptr<expression> rhs)
        : _op{ op }, _lhs{ std::move(lhs) }, _rhs{ std::move(rhs) }
    {
        _set_ast_info(parse);
    }

    void binary_expression::print(std::ostream & os, print_context ctx) const
    {
        os << styles::def << ctx << styles::rule_name << "binary-expression";
        print_address_range(os, this);
        os << '\n';

        auto type_ctx = ctx.make_branch(false);
        os << styles::def << type_ctx << styles::subrule_name << "type:\n";
        get_type()->print(os, type_ctx.make_branch(true));

        auto lhs_ctx = ctx.make_branch(false);
        os << styles::def << lhs_ctx << styles::subrule_name << "lhs:\n";
        _lhs->print(os, lhs_ctx.make_branch(true));

        auto operator_ctx = ctx.make_branch(false);
        os << styles::def << operator_ctx << styles::subrule_name << "operator: " << styles::string_value << lexer::token_types[+_op.type] << '\n';

        auto rhs_ctx = ctx.make_branch(false);
        os << styles::def << rhs_ctx << styles::subrule_name << "rhs:\n";
        _rhs->print(os, rhs_ctx.make_branch(true));

        auto call_expr_ctx = ctx.make_branch(true);
        os << styles::def << call_expr_ctx << styles::subrule_name << "resolved expression:\n";
        _call_expression->print(os, call_expr_ctx.make_branch(true));
    }

    statement_ir binary_expression::_codegen_ir(ir_generation_context & ctx) const
    {
        // TODO: there should be a different order of evaluation for right-associative and left-associative
        // (...probably)
        return _call_expression->codegen_ir(ctx);
    }
}
}
