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

#include <reaver/variant.h>

#include "../lexer/token.h"
#include "scope.h"
#include "ir_context.h"

namespace reaver
{
    namespace vapor
    {
        namespace codegen { inline namespace _v1
        {
            namespace ir
            {
                struct variable_type;
            }
        }}

        namespace analyzer { inline namespace _v1
        {
            class function;

            class type : public std::enable_shared_from_this<type>
            {
            public:
                type() : _lex_scope{ std::make_shared<scope>() }
                {
                }

                type(std::shared_ptr<scope> outer_scope) : _lex_scope{ outer_scope->clone_for_decl() }
                {
                    assert(0);
                }

                virtual ~type() = default;

                virtual std::shared_ptr<function> get_overload(lexer::token_type, std::shared_ptr<type>) const
                {
                    return nullptr;
                }

                virtual std::shared_ptr<function> get_overload(lexer::token_type, std::vector<std::shared_ptr<type>>) const
                {
                    return nullptr;
                }

                virtual std::string explain() const = 0;

                virtual std::shared_ptr<scope> get_scope() const
                {
                    return _lex_scope;
                }

                std::shared_ptr<codegen::ir::variable_type> codegen_type(ir_generation_context & ctx) const
                {
                    if (!_codegen_t)
                    {
                        _codegen_t = _codegen_type(ctx);
                    }

                    return *_codegen_t;
                }

            private:
                virtual std::shared_ptr<codegen::ir::variable_type> _codegen_type(ir_generation_context &) const = 0;

                std::shared_ptr<scope> _lex_scope;
                mutable optional<std::shared_ptr<codegen::ir::variable_type>> _codegen_t;
            };

            class type_type : public type
            {
            public:
                virtual std::string explain() const override
                {
                    return "type";
                }

            private:
                virtual std::shared_ptr<codegen::ir::variable_type> _codegen_type(ir_generation_context &) const override;
            };

            // these here are currently kinda silly
            // will get less silly and properly separated once typeclasses are a thing

            inline std::shared_ptr<function> resolve_overload(const std::shared_ptr<type> & lhs, const std::shared_ptr<type> & rhs, lexer::token_type op, std::shared_ptr<scope> in_scope)
            {
                auto overload = lhs->get_overload(op, rhs);
                if (overload)
                {
                    return overload;
                }

                logger::dlog() << lhs << " " << lexer::token_types[+op] << " " << rhs << " = ?";
                assert(0);
            }

            inline std::shared_ptr<function> resolve_overload(const std::shared_ptr<type> & base_expr, lexer::token_type bracket_type, std::vector<std::shared_ptr<type>> arguments, std::shared_ptr<scope> in_scope)
            {
                auto overload = base_expr->get_overload(bracket_type, arguments);
                if (overload)
                {
                    return overload;
                }

                assert(0);
            }

            std::shared_ptr<type> make_integer_type();

            inline const auto & builtin_types()
            {
                struct builtin_types_t
                {
                    using member_t = std::shared_ptr<class type>;

                    member_t type;
                    member_t integer;
                };

                static auto builtins = []{
                    builtin_types_t builtins;

                    builtins.type = std::make_shared<type_type>();
                    builtins.integer = make_integer_type();

                    return builtins;
                }();

                return builtins;
            }
        }}
    }
}

