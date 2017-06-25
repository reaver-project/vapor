/**
 * Vapor Compiler Licence
 *
 * Copyright © 2014, 2016-2017 Michał "Griwes" Dominiak
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

#include "../expressions/expression.h"
#include "../expressions/member.h"
#include "../expressions/type.h"
#include "../symbol.h"
#include "statement.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    enum class declaration_type
    {
        variable,
        member
    };

    class declaration : public statement
    {
    public:
        declaration(const parser::declaration & parse, scope * old_scope, scope * new_scope, declaration_type decl_type);

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

        auto declared_member() const
        {
            assert(_declared_member);
            return _declared_member.get().get();
        }

        auto initializer_expression() const
        {
            return fmap(_init_expr, [](auto && expr) { return expr.get(); });
        }

        virtual void print(std::ostream & os, print_context ctx) const override;

    private:
        virtual future<> _analyze(analysis_context & ctx) override;
        virtual std::unique_ptr<statement> _clone_with_replacement(replacements & repl) const override;
        virtual future<statement *> _simplify(simplification_context & ctx) override;
        virtual statement_ir _codegen_ir(ir_generation_context & ctx) const override;

        const parser::declaration & _parse;
        std::u32string _name;
        symbol * _declared_symbol;
        optional<std::unique_ptr<expression>> _type_specifier;
        optional<std::unique_ptr<expression>> _init_expr;
        optional<std::unique_ptr<member_expression>> _declared_member;
        declaration_type _type;
    };

    inline std::unique_ptr<declaration> preanalyze_declaration(const parser::declaration & parse, scope *& lex_scope)
    {
        auto old_scope = lex_scope;
        lex_scope = old_scope->clone_for_decl();
        return std::make_unique<declaration>(parse, old_scope, lex_scope, declaration_type::variable);
    }

    inline std::unique_ptr<declaration> preanalyze_member_declaration(const parser::declaration & parse, scope * lex_scope)
    {
        return std::make_unique<declaration>(parse, lex_scope, lex_scope, declaration_type::member);
    }
}
}
