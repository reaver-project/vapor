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

#include "../parser/binary_expression.h"
#include "expression.h"

namespace reaver
{
    namespace vapor
    {
        namespace analyzer { inline namespace _v1
        {
            class function;

            class binary_expression : public expression
            {
            public:
                binary_expression(const parser::binary_expression & parse, scope * lex_scope) : _parse{ parse }, _scope{ lex_scope }, _op{ _parse.op },
                    _lhs{ preanalyze_expression(_parse.lhs, lex_scope) },
                    _rhs{ preanalyze_expression(_parse.rhs, lex_scope) }
                {
                }

                virtual void print(std::ostream & os, std::size_t indent) const override;

            private:
                binary_expression(const binary_expression & other) : _parse{ other._parse }, _op{ other._op }, _overload{ other._overload }
                {
                }

                virtual future<> _analyze() override;
                virtual std::unique_ptr<expression> _clone_expr_with_replacement(replacements &) const override;
                virtual future<expression *> _simplify_expr(optimization_context &) override;
                virtual statement_ir _codegen_ir(ir_generation_context &) const override;

                const parser::binary_expression & _parse;
                scope * _scope = nullptr;
                lexer::token _op;
                std::unique_ptr<expression> _lhs;
                std::unique_ptr<expression> _rhs;
                function * _overload;
            };

            inline std::unique_ptr<binary_expression> preanalyze_binary_expression(const parser::binary_expression & parse, scope * lex_scope)
            {
                return std::make_unique<binary_expression>(parse, lex_scope);
            }
        }}
    }
}

