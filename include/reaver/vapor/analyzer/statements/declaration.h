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
        declaration(ast_node parse,
            std::u32string name,
            std::optional<std::unique_ptr<expression>> init_expr,
            std::optional<std::unique_ptr<expression>> type_specifier,
            scope * scope,
            declaration_type decl_type);

        const auto & name() const
        {
            return _name;
        }

        auto declared_symbol() const
        {
            return _declared_symbol;
        }

        auto declared_member() const
        {
            assert(_declared_member);
            return _declared_member->get();
        }

        auto initializer_expression() const
        {
            return fmap(_init_expr, [](auto && expr) { return expr.get(); });
        }

        virtual void print(std::ostream & os, print_context ctx) const override;

    private:
        virtual future<> _analyze(analysis_context & ctx) override;
        virtual std::unique_ptr<statement> _clone_with_replacement(replacements & repl) const override;
        virtual future<statement *> _simplify(recursive_context ctx) override;
        virtual statement_ir _codegen_ir(ir_generation_context & ctx) const override;

        std::u32string _name;
        symbol * _declared_symbol;
        std::optional<std::unique_ptr<expression>> _type_specifier;
        std::optional<std::unique_ptr<expression>> _init_expr;
        std::optional<std::unique_ptr<member_expression>> _declared_member;
        declaration_type _type;
    };
}
}

namespace reaver::vapor::parser
{
inline namespace _v1
{
    struct declaration;
}
}
namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    std::unique_ptr<declaration> preanalyze_declaration(const parser::declaration & parse, scope *& lex_scope);
    std::unique_ptr<declaration> preanalyze_member_declaration(const parser::declaration & parse, scope * lex_scope);
}
}
