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
#include "expression.h"
#include "block.h"
#include "symbol.h"
#include "argument_list.h"

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
                overload_set_type(scope * lex_scope) : type{ lex_scope }
                {
                }

                void add_function(function * fn);

                virtual std::string explain() const override
                {
                    return "overload set (TODO: add location and member info)";
                }

                virtual future<function *> get_overload(lexer::token_type bracket, std::vector<const type *> args) const override;

            private:
                virtual void _codegen_type(ir_generation_context &) const override;

                mutable std::mutex _functions_lock;
                std::vector<function *> _functions;
            };

            class function_declaration;

            class overload_set : public variable, public std::enable_shared_from_this<overload_set>
            {
            public:
                overload_set(scope * lex_scope) : _type{ std::make_unique<overload_set_type>(lex_scope) }
                {
                }

                void add_function(function_declaration * fn);

                virtual type * get_type() const override
                {
                    return _type.get();
                }

            private:
                virtual std::unique_ptr<variable> _clone_with_replacement(replacements &) const override;
                virtual variable_ir _codegen_ir(ir_generation_context &) const override;

                std::vector<function_declaration *> _overloads;
                std::unique_ptr<overload_set_type> _type;
            };

            class function_declaration : public statement
            {
            public:
                function_declaration(const parser::function & parse, scope * parent_scope)
                    : _parse{ parse }, _scope{ parent_scope->clone_local() }
                {
                    fmap(parse.arguments, [&](auto && arglist) {
                        _argument_list = preanalyze_argument_list(arglist, _scope.get());
                        return unit{};
                    });
                    _scope->close();

                    _return_type = fmap(_parse.return_type, [&](auto && ret_type) {
                        return preanalyze_expression(ret_type, _scope.get());
                    });
                    _body = preanalyze_block(*_parse.body, _scope.get(), true);
                    std::shared_ptr<overload_set> keep_count;
                    auto symbol = parent_scope->get_or_init(_parse.name.string, [&]{
                        keep_count = std::make_shared<overload_set>(_scope.get());
                        return make_symbol(_parse.name.string, keep_count.get());
                    });

                    _overload_set = dynamic_cast<overload_set *>(symbol->get_variable())->shared_from_this();
                }

                function * get_function() const
                {
                    return _function.get();
                }

                virtual void print(std::ostream & os, std::size_t indent) const override;

            private:
                virtual future<> _analyze() override;

                virtual std::unique_ptr<statement> _clone_with_replacement(replacements &) const override
                {
                    return make_null_statement();
                }

                virtual future<statement *> _simplify(optimization_context &) override;
                virtual statement_ir _codegen_ir(ir_generation_context &) const override;

                const parser::function & _parse;
                argument_list _argument_list;

                optional<std::unique_ptr<expression>> _return_type;
                std::unique_ptr<block> _body;
                std::unique_ptr<scope> _scope;
                std::unique_ptr<function> _function;
                std::shared_ptr<overload_set> _overload_set;
            };

            inline void overload_set::add_function(function_declaration * fn)
            {
                _type->add_function(fn->get_function());
                _overloads.push_back(std::move(fn));
            }

            inline std::unique_ptr<function_declaration> preanalyze_function(const parser::function & func, scope * & lex_scope)
            {
                return std::make_unique<function_declaration>(func, lex_scope);
            }
        }}
    }
}

