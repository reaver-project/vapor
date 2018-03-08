/**
 * Vapor Compiler Licence
 *
 * Copyright © 2016-2018 Michał "Griwes" Dominiak
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

#include "vapor/analyzer/statements/declaration.h"
#include "vapor/parser/declaration.h"
#include "vapor/parser/expr.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    std::unique_ptr<declaration> _preanalyze_declaration(precontext & ctx,
        const parser::declaration & parse,
        scope * old_scope,
        scope * new_scope,
        declaration_type type)
    {
        switch (type)
        {
            case declaration_type::variable:
                assert(parse.rhs);
                break;

            case declaration_type::member:
                assert(parse.type_expression || parse.rhs);
                break;
        }

        auto ret = std::make_unique<declaration>(make_node(parse),
            parse.identifier.value.string,
            fmap(parse.rhs, [&](auto && expr) { return preanalyze_expression(ctx, expr, old_scope); }),
            fmap(parse.type_expression, [&](auto && expr) { return preanalyze_expression(ctx, expr, old_scope); }),
            new_scope,
            type);

        if (old_scope != new_scope)
        {
            new_scope->close();
        }

        return ret;
    }

    std::unique_ptr<declaration> preanalyze_declaration(precontext & ctx, const parser::declaration & parse, scope *& lex_scope)
    {
        auto old_scope = lex_scope;
        lex_scope = old_scope->clone_for_decl();
        return _preanalyze_declaration(ctx, parse, old_scope, lex_scope, declaration_type::variable);
    }

    std::unique_ptr<declaration> preanalyze_member_declaration(precontext & ctx, const parser::declaration & parse, scope * lex_scope)
    {
        return _preanalyze_declaration(ctx, parse, lex_scope, lex_scope, declaration_type::member);
    }

    declaration::declaration(ast_node parse,
        std::u32string name,
        std::optional<std::unique_ptr<expression>> init_expr,
        std::optional<std::unique_ptr<expression>> type_specifier,
        scope * scope,
        declaration_type decl_type)
        : _name{ std::move(name) }, _type_specifier{ std::move(type_specifier) }, _init_expr{ std::move(init_expr) }, _type{ decl_type }
    {
        _set_ast_info(parse);

        auto symbol = make_symbol(_name);
        _declared_symbol = symbol.get();
        if (!scope->init(_name, std::move(symbol)))
        {
            assert(0);
        }
    }

    void declaration::print(std::ostream & os, print_context ctx) const
    {
        os << styles::def << ctx << styles::rule_name << "declaration";
        print_address_range(os, this);
        os << ' ' << styles::string_value << utf8(_name) << '\n';

        auto type_ctx = ctx.make_branch(!_init_expr);
        os << styles::def << type_ctx << styles::subrule_name << "type of symbol:\n";
        _declared_symbol->get_type()->print(os, type_ctx.make_branch(true));

        if (_init_expr)
        {
            auto init_expr_ctx = ctx.make_branch(true);
            os << styles::def << init_expr_ctx << styles::subrule_name << "initializer expression:\n";
            _init_expr.value()->print(os, init_expr_ctx.make_branch(true));
        }
    }

    statement_ir declaration::_codegen_ir(ir_generation_context & ctx) const
    {
        if (_declared_symbol->get_type() == builtin_types().type.get())
        {
            return {};
        }

        assert(_type == declaration_type::variable);
        auto ir = _init_expr.value()->codegen_ir(ctx);

        if (ir.back().result.index() == 0)
        {
            auto var = std::get<std::shared_ptr<codegen::ir::variable>>(ir.back().result);
            ir.back().declared_variable = var;
            var->name = _name;
            var->temporary = false;
        }
        return ir;
    }
}
}
