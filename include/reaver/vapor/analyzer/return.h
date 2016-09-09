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

namespace reaver
{
    namespace vapor
    {
        namespace analyzer { inline namespace _v1
        {
            class return_statement : public statement, public std::enable_shared_from_this<return_statement>
            {
            public:
                return_statement(const parser::return_expression & parse, std::shared_ptr<scope> lex_scope) : _parse{ parse }
                {
                    _value_expr = preanalyze_expression(parse.return_value, lex_scope);
                }

                virtual std::vector<std::shared_ptr<const return_statement>> get_returns() const override
                {
                    return { shared_from_this() };
                }

                std::shared_ptr<type> get_returned_type() const
                {
                    return _value_expr->get_type();
                }

                virtual void print(std::ostream & os, std::size_t indent) const override
                {
                    auto in = std::string(indent, ' ');
                    os << in << "return statement at " << _parse.range << '\n';
                    os << in << "return value expression:\n";
                    os << in << "{\n";
                    _value_expr->print(os, indent + 4);
                    os << in << "}\n";
                }

            private:
                virtual future<> _analyze() override
                {
                    return _value_expr->analyze();
                }

                const parser::return_expression & _parse;
                std::shared_ptr<expression> _value_expr;
            };

            inline std::shared_ptr<return_statement> preanalyze_return(const parser::return_expression & parse, std::shared_ptr<scope> lex_scope)
            {
                return std::make_shared<return_statement>(parse, lex_scope);
            }
        }}
    }
}

