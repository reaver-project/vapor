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

#include <memory>

#include "../parser/lambda_expression.h"
#include "scope.h"
#include "statement.h"
#include "block.h"
#include "function.h"

namespace reaver
{
    namespace vapor
    {
        namespace analyzer { inline namespace _v1
        {
            class closure_type : public type
            {
            public:
                closure_type(std::shared_ptr<scope> lex_scope, std::shared_ptr<expression> closure, std::shared_ptr<function> fn) : type{ std::move(lex_scope) }, _closure{ std::move(closure) }, _function{ std::move(fn) }
                {
                }

                virtual std::string explain() const override
                {
                    return "closure (TODO: location)";
                }

                virtual std::shared_ptr<function> get_overload(lexer::token_type bracket, std::vector<std::shared_ptr<type>> args) const override
                {
                    if (_function->arguments() == args)
                    {
                        return _function;
                    }

                    return nullptr;
                }

            private:
                virtual std::shared_ptr<codegen::ir::variable_type> _codegen_type(ir_generation_context &) const override;

                std::shared_ptr<expression> _closure;
                std::shared_ptr<function> _function;
            };

            class closure : public expression
            {
            public:
                closure(const parser::lambda_expression & parse, std::shared_ptr<scope> lex_scope) : _parse{ parse }, _scope{ lex_scope->clone_local() }
                {
                    _scope->close();
                    _body = preanalyze_block(parse.body, _scope, true);
                }

                auto & parse() const
                {
                    return _parse;
                }

                virtual void print(std::ostream & os, std::size_t indent) const override;

            private:
                virtual future<> _analyze() override;
                virtual future<std::shared_ptr<expression>> _simplify_expr(optimization_context &) override;
                virtual statement_ir _codegen_ir(ir_generation_context &) const override;

                const parser::lambda_expression & _parse;
                std::shared_ptr<scope> _scope;
                std::shared_ptr<block> _body;
                std::shared_ptr<type> _type;
            };

            inline std::shared_ptr<closure> preanalyze_closure(const parser::lambda_expression & parse, std::shared_ptr<scope> lex_scope)
            {
                return std::make_shared<closure>(parse, std::move(lex_scope));
            }
        }}
    }
}

