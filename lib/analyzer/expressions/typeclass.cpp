/**
 * Vapor Compiler Licence
 *
 * Copyright © 2017-2019 Michał "Griwes" Dominiak
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

#include "vapor/analyzer/expressions/typeclass.h"
#include "vapor/analyzer/semantic/symbol.h"
#include "vapor/analyzer/statements/function.h"
#include "vapor/parser/expr.h"
#include "vapor/parser/typeclass.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    std::unique_ptr<typeclass_literal> preanalyze_typeclass_literal(precontext & ctx, const parser::typeclass_literal & parse, scope * lex_scope)
    {
        return std::make_unique<typeclass_literal>(make_node(parse), make_typeclass(ctx, parse, lex_scope));
    }

    typeclass_literal::typeclass_literal(ast_node parse, std::unique_ptr<typeclass> type)
        : expression{ builtin_types().typeclass.get() }, _instance_template{ std::move(type) }
    {
        _set_ast_info(parse);
    }

    typeclass_literal::~typeclass_literal() = default;

    void typeclass_literal::print(std::ostream & os, print_context ctx) const
    {
        os << styles::def << ctx << styles::rule_name << "typeclass-literal";
        print_address_range(os, this);
        os << '\n';

        auto tc_ctx = ctx.make_branch(_instance_template->get_member_function_decls().empty());
        os << styles::def << tc_ctx << styles::subrule_name << "defined typeclass:\n";
        _instance_template->print(os, tc_ctx.make_branch(true));

        if (_instance_template->get_member_function_decls().size())
        {
            auto decl_ctx = ctx.make_branch(true);
            os << styles::def << decl_ctx << styles::subrule_name << "member function declarations:\n";

            std::size_t idx = 0;
            for (auto && member : _instance_template->get_member_function_decls())
            {
                member->print(os, decl_ctx.make_branch(++idx == _instance_template->get_member_function_decls().size()));
            }
        }
    }

    future<> typeclass_literal::_analyze(analysis_context & ctx)
    {
        assert(0);
        /*
         return when_all(fmap(_instance_template->get_member_function_decls(), [&](auto && decl) {
             decl->set_template_parameters(_instance_template->get_template_parameters());
             return decl->analyze(ctx);
         }));*/
    }

    std::unique_ptr<expression> typeclass_literal::_clone_expr_with_replacement(replacements & repl) const
    {
        assert(0);
    }

    future<expression *> typeclass_literal::_simplify_expr(recursive_context ctx)
    {
        assert(0);
    }

    statement_ir typeclass_literal::_codegen_ir(ir_generation_context & ctx) const
    {
        assert(0);
    }

    typeclass_literal_instance::typeclass_literal_instance(typeclass * tc, std::vector<expression *> arguments)
        : _instance_type{ std::make_unique<typeclass_instance_type>(tc, std::move(arguments)) }
    {
    }
}
}
