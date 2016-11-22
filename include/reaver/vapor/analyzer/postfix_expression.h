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

#pragma once

#include "expression.h"

namespace reaver::vapor::parser
{
inline namespace _v1
{
    struct postfix_expression;
}
}

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    class scope;
    class function;

    class postfix_expression : public expression
    {
    public:
        postfix_expression(const parser::postfix_expression & parse, scope * lex_scope);

        virtual void print(std::ostream & os, std::size_t indent) const override;
        virtual variable * get_variable() const override;

    private:
        postfix_expression(const postfix_expression & other) : _parse{ other._parse }, _brace{ other._brace }, _overload{ other._overload }
        {
        }

        virtual future<> _analyze(analysis_context &) override;
        virtual std::unique_ptr<expression> _clone_expr_with_replacement(replacements &) const override;
        virtual future<expression *> _simplify_expr(simplification_context &) override;
        virtual statement_ir _codegen_ir(ir_generation_context &) const override;

        const parser::postfix_expression & _parse;
        scope * _scope = nullptr;
        std::unique_ptr<expression> _base_expr;
        optional<lexer::token_type> _brace;
        std::vector<std::unique_ptr<expression>> _arguments;
        function * _overload;
    };

    inline std::unique_ptr<postfix_expression> preanalyze_postfix_expression(const parser::postfix_expression & parse, scope * lex_scope)
    {
        return std::make_unique<postfix_expression>(parse, lex_scope);
    }
}
}
