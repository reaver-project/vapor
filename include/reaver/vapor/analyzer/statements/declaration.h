/**
 * Vapor Compiler Licence
 *
 * Copyright © 2014, 2016 Michał "Griwes" Dominiak
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

#pragma once

#include <memory>

#include "../../parser/declaration.h"
#include "../expressions/expression.h"
#include "../symbol.h"
#include "../variables/variable.h"
#include "statement.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    class declaration : public statement
    {
    public:
        declaration(const parser::declaration & parse, scope * old_scope, scope * new_scope) : _parse{ parse }, _name{ parse.identifier.string }
        {
            _init_expr = preanalyze_expression(_parse.rhs, old_scope);
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

        const auto & name() const
        {
            return _name;
        }

        const auto & parse() const
        {
            return _parse;
        }

        auto declared_symbol() const
        {
            return _declared_symbol;
        }

        auto initializer_expression() const
        {
            return _init_expr.get();
        }

        virtual void print(std::ostream & os, std::size_t indent) const override
        {
            auto in = std::string(indent, ' ');
            os << in << "declaration of `" << utf8(_name) << "` at " << _parse.range << '\n';
            os << in << "type of symbol: " << _declared_symbol->get_type()->explain() << '\n';
            os << in << "initializer expression:\n";
            os << in << "{\n";
            _init_expr->print(os, indent + 4);
            os << in << "}\n";
        }

    private:
        virtual future<> _analyze(analysis_context & ctx) override
        {
            return _init_expr->analyze(ctx).then([&] { _declared_symbol->set_variable(_init_expr->get_variable()); });
        }

        virtual std::unique_ptr<statement> _clone_with_replacement(replacements & repl) const override
        {
            return _init_expr->clone_expr_with_replacement(repl);
        }

        virtual future<statement *> _simplify(simplification_context & ctx) override
        {
            return _init_expr->simplify_expr(ctx)
                .then([&](auto && simplified) { replace_uptr(_init_expr, simplified, ctx); })
                .then([&]() { return _declared_symbol->simplify(ctx); })
                .then([&]() -> statement * { return this; });
        }

        virtual statement_ir _codegen_ir(ir_generation_context & ctx) const override
        {
            auto ir = _init_expr->codegen_ir(ctx);

            if (ir.back().result.index() == 0)
            {
                auto var = get<std::shared_ptr<codegen::ir::variable>>(ir.back().result);
                ir.back().declared_variable = var;
                var->name = _name;
                var->temporary = false;
            }
            return ir;
        }

        const parser::declaration & _parse;
        std::u32string _name;
        symbol * _declared_symbol;
        std::unique_ptr<expression> _init_expr;
    };

    inline std::unique_ptr<declaration> preanalyze_declaration(const parser::declaration & parse, scope *& lex_scope)
    {
        auto old_scope = lex_scope;
        lex_scope = old_scope->clone_for_decl();
        return std::make_unique<declaration>(parse, old_scope, lex_scope);
    }
}
}
