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

#include "../parser/return_expression.h"
#include "statement.h"
#include "expression.h"
#include "helpers.h"

namespace reaver
{
    namespace vapor
    {
        namespace analyzer { inline namespace _v1
        {
            class return_statement : public statement
            {
            public:
                return_statement(const parser::return_expression & parse, scope * lex_scope) : _parse{ parse }
                {
                    _value_expr = preanalyze_expression(parse.return_value, lex_scope);
                }

                virtual std::vector<const return_statement *> get_returns() const override
                {
                    return { this };
                }

                type * get_returned_type() const
                {
                    return _value_expr->get_type();
                }

                variable * get_returned_variable() const
                {
                    return _value_expr->get_variable();
                }

                virtual void print(std::ostream & os, std::size_t indent) const override;

            private:
                virtual future<> _analyze() override
                {
                    return _value_expr->analyze();
                }

                return_statement(const return_statement & other) : _parse{ other._parse }
                {
                }

                virtual std::unique_ptr<statement> _clone_with_replacement(replacements & repl) const override
                {
                    auto ret = std::unique_ptr<return_statement>(new return_statement(*this));

                    ret->_value_expr = _value_expr->clone_expr_with_replacement(repl);

                    return ret;
                }

                virtual future<statement *> _simplify(optimization_context & ctx) override
                {
                    return _value_expr->simplify_expr(ctx).then([&](auto && simplified) -> statement * {
                            replace_uptr(_value_expr, simplified, ctx);
                            return this;
                        });
                }

                virtual statement_ir _codegen_ir(ir_generation_context &) const override;

                const parser::return_expression & _parse;
                std::unique_ptr<expression> _value_expr;
            };

            inline std::unique_ptr<return_statement> preanalyze_return(const parser::return_expression & parse, scope * lex_scope)
            {
                return std::make_unique<return_statement>(parse, lex_scope);
            }
        }}
    }
}

