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
#include <numeric>

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
                closure_type(scope * lex_scope, expression * closure, std::unique_ptr<function> fn) : type{ lex_scope }, _closure{ std::move(closure) }, _function{ std::move(fn) }
                {
                }

                virtual std::string explain() const override
                {
                    return "closure (TODO: location)";
                }

                virtual function * get_overload(lexer::token_type bracket, std::vector<const type *> args) const override
                {
                    if (std::inner_product(
                            args.begin(), args.end(),
                            _function->arguments().begin(),
                            true,
                            std::logical_and<>(),
                            std::equal_to<>()
                        ))
                    {
                        return _function.get();
                    }

                    return nullptr;
                }

            private:
                virtual std::shared_ptr<codegen::ir::variable_type> _codegen_type(ir_generation_context &) const override;

                expression * _closure;
                std::unique_ptr<function> _function;
            };

            class closure : public expression
            {
            public:
                closure(const parser::lambda_expression & parse, scope * lex_scope) : _parse{ parse }, _scope{ lex_scope->clone_local() }
                {
                    _scope->close();
                    _body = preanalyze_block(parse.body, _scope.get(), true);
                }

                auto & parse() const
                {
                    return _parse;
                }

                virtual void print(std::ostream & os, std::size_t indent) const override;

            private:
                virtual future<> _analyze() override;
                virtual future<expression *> _simplify_expr(optimization_context &) override;
                virtual statement_ir _codegen_ir(ir_generation_context &) const override;

                const parser::lambda_expression & _parse;
                std::unique_ptr<scope> _scope;
                std::unique_ptr<block> _body;
                std::unique_ptr<type> _type;
            };

            inline std::unique_ptr<closure> preanalyze_closure(const parser::lambda_expression & parse, scope * lex_scope)
            {
                return std::make_unique<closure>(parse, lex_scope);
            }
        }}
    }
}

