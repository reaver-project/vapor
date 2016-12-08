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

#include "vapor/analyzer/statements/declaration.h"
#include "vapor/parser/declaration.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    declaration::declaration(const parser::declaration & parse, scope * old_scope, scope * new_scope, declaration_type decl_type)
        : _parse{ parse }, _name{ parse.identifier.string }, _type{ decl_type }
    {
        switch (_type)
        {
            case declaration_type::variable:
                assert(_parse.rhs);
                break;

            case declaration_type::member:
                assert(_parse.type_expression || _parse.rhs);
                break;
        }

        _type_specifier = fmap(_parse.type_expression, [&](auto && expr) { return preanalyze_expression(expr, old_scope); });
        _init_expr = fmap(_parse.rhs, [&](auto && expr) { return preanalyze_expression(expr, old_scope); });

        auto symbol = make_symbol(_name);
        _declared_symbol = symbol.get();
        if (!new_scope->init(_name, std::move(symbol)))
        {
            assert(0);
        }

        if (old_scope != new_scope)
        {
            new_scope->close();
        }
    }

    void declaration::print(std::ostream & os, std::size_t indent) const
    {
        auto in = std::string(indent, ' ');
        os << in << "declaration of `" << utf8(_name) << "` at " << _parse.range << '\n';
        os << in << "type of symbol: " << _declared_symbol->get_type()->explain() << '\n';
        os << in << "initializer expression:\n";
        os << in << "{\n";
        fmap(_init_expr, [&](auto && expr) {
            expr->print(os, indent + 4);
            return unit{};
        });
        os << in << "}\n";
    }

    statement_ir declaration::_codegen_ir(ir_generation_context & ctx) const
    {
        assert(_type == declaration_type::variable);
        auto ir = _init_expr.get()->codegen_ir(ctx);

        if (ir.back().result.index() == 0)
        {
            auto var = get<std::shared_ptr<codegen::ir::variable>>(ir.back().result);
            ir.back().declared_variable = var;
            var->name = _name;
            var->temporary = false;
        }
        return ir;
    }
}
}
