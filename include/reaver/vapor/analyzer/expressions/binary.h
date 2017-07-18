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

#pragma once

#include "../../parser/binary_expression.h"
#include "expression.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    class function;

    class binary_expression : public expression
    {
    public:
        binary_expression(const parser::binary_expression & parse, scope * lex_scope)
            : _parse{ parse },
              _scope{ lex_scope },
              _op{ _parse.op },
              _lhs{ preanalyze_expression(_parse.lhs, lex_scope) },
              _rhs{ preanalyze_expression(_parse.rhs, lex_scope) }
        {
        }

        virtual void print(std::ostream & os, print_context ctx) const override;

        const expression * get_lhs() const
        {
            return _lhs.get();
        }

        const expression * get_rhs() const
        {
            return _rhs.get();
        }

        lexer::token_type get_operator() const
        {
            return _op.type;
        }

        const auto & parse() const
        {
            return _parse;
        }

    private:
        binary_expression(const binary_expression & other) : _parse{ other._parse }, _op{ other._op }
        {
        }

        virtual expression * _get_replacement() override
        {
            return _call_expression->_get_replacement();
        }

        virtual const expression * _get_replacement() const override
        {
            return _call_expression->_get_replacement();
        }

        virtual future<> _analyze(analysis_context &) override;
        virtual std::unique_ptr<expression> _clone_expr_with_replacement(replacements &) const override;
        virtual future<expression *> _simplify_expr(simplification_context &) override;
        virtual statement_ir _codegen_ir(ir_generation_context &) const override;

        const parser::binary_expression & _parse;
        scope * _scope = nullptr;
        lexer::token _op;
        std::unique_ptr<expression> _lhs;
        std::unique_ptr<expression> _rhs;
        std::unique_ptr<expression> _call_expression;
    };

    inline std::unique_ptr<binary_expression> preanalyze_binary_expression(const parser::binary_expression & parse, scope * lex_scope)
    {
        return std::make_unique<binary_expression>(parse, lex_scope);
    }
}
}
