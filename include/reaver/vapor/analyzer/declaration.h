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

#include "../parser/declaration.h"
#include "symbol.h"
#include "expression.h"
#include "variable.h"
#include "statement.h"

namespace reaver
{
    namespace vapor
    {
        namespace analyzer { inline namespace _v1
        {
            class declaration : public statement
            {
            public:
                declaration(const parser::declaration & parse, std::shared_ptr<scope> old_scope, std::shared_ptr<scope> new_scope)
                    : _parse{ parse }, _name{ parse.identifier.string }
                {
                    _init_expr = preanalyze_expression(_parse.rhs, old_scope);
                    _declared_symbol = make_symbol(_name);
                    if (!new_scope->init(_name, _declared_symbol))
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
                    return _init_expr;
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
                virtual future<> _analyze() override
                {
                    return _init_expr->analyze().then([&]{
                        _declared_symbol->set_variable(_init_expr->get_variable());
                    });
                }

                virtual statement_ir _codegen_ir() const override
                {
                    auto declared_var = codegen::ir::make_variable(_declared_symbol->get_type()->codegen_type(), _name);

                    auto instructions = _init_expr->codegen_ir();
                    instructions.insert(instructions.begin(), codegen::ir::instruction{
                        none,
                        { declared_var },
                        { boost::typeindex::type_id<codegen::ir::declaration_instruction>() },
                        {},
                        declared_var
                    });
                    return instructions;
                }

                const parser::declaration & _parse;
                std::u32string _name;
                std::shared_ptr<symbol> _declared_symbol;
                std::shared_ptr<expression> _init_expr;
            };

            inline std::shared_ptr<declaration> preanalyze_declaration(const parser::declaration & parse, std::shared_ptr<scope> & lex_scope)
            {
                auto old_scope = lex_scope;
                lex_scope = old_scope->clone_for_decl();
                return std::make_shared<declaration>(parse, old_scope, lex_scope);
            }
        }}
    }
}

