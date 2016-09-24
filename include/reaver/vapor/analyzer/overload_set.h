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

#include "../parser/function.h"
#include "variable.h"
#include "statement.h"
#include "block.h"
#include "symbol.h"

namespace reaver
{
    namespace vapor
    {
        namespace analyzer { inline namespace _v1
        {
            class function;

            // TODO: combined_overload_set and combined_overload_set_type
            // those need to be distinct from normal ones for code generation reasons (SCOPES!)
            // this is not a crucial feature *right now*, but it will be, and rather soon
            // this is just a note so that I remember why the hell I can't use the existing types
            //
            // random bikeshedding thoughts:
            // actually I might be able to do this differently, on the function level
            // i.e. create a proxy_function that generates a function in current scope that calls
            // a function that's in a different scope
            // but that's going to be funny

            class overload_set_type : public type
            {
            public:
                overload_set_type(std::shared_ptr<scope> lex_scope) : type{ std::move(lex_scope) }
                {
                }

                void add_function(std::shared_ptr<function> fn);

                virtual std::string explain() const override
                {
                    return "overload set (TODO: add location and member info)";
                }

                virtual std::shared_ptr<function> get_overload(lexer::token_type bracket, std::vector<std::shared_ptr<type>> args) const override;

            private:
                virtual std::shared_ptr<codegen::ir::variable_type> _codegen_type(ir_generation_context &) const override;

                std::vector<std::shared_ptr<function>> _functions;
            };

            class function_declaration;

            class overload_set : public variable
            {
            public:
                overload_set(std::shared_ptr<scope> lex_scope) : _type{ std::make_shared<overload_set_type>(std::move(lex_scope)) }
                {
                }

                void add_function(std::shared_ptr<function_declaration> fn);

                virtual std::shared_ptr<type> get_type() const override
                {
                    return _type;
                }

            private:
                virtual variable_ir _codegen_ir(ir_generation_context &) const override;

                std::vector<std::shared_ptr<function_declaration>> _overloads;
                std::shared_ptr<overload_set_type> _type;
            };

            class function_declaration : public statement, public std::enable_shared_from_this<function_declaration>
            {
            public:
                function_declaration(const parser::function & parse, std::shared_ptr<scope> scope)
                    : _parse{ parse }, _scope{ scope }
                {
                    assert(!_parse.arguments);

                    _body = preanalyze_block(*_parse.body, scope, true);
                    auto set = _scope->get_or_init(_parse.name.string,
                        [&]{ return make_symbol(_parse.name.string, std::make_shared<overload_set>(scope)); });
                }

                std::shared_ptr<function> get_function() const
                {
                    return _function;
                }

                virtual void print(std::ostream & os, std::size_t indent) const override;

            private:
                virtual future<> _analyze() override;
                virtual statement_ir _codegen_ir(ir_generation_context &) const override;

                const parser::function & _parse;

                std::shared_ptr<block> _body;
                std::shared_ptr<scope> _scope;
                std::shared_ptr<function> _function;
            };

            inline void overload_set::add_function(std::shared_ptr<function_declaration> fn)
            {
                _type->add_function(fn->get_function());
                _overloads.push_back(std::move(fn));
            }

            inline std::shared_ptr<function_declaration> preanalyze_function(const parser::function & func, std::shared_ptr<scope> & lex_scope)
            {
                return std::make_shared<function_declaration>(func, lex_scope);
            }
        }}
    }
}

