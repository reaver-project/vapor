/**
 * Vapor Compiler Licence
 *
 * Copyright © 2014-2016 Michał "Griwes" Dominiak
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

#include <reaver/prelude/monad.h>

#include "variable.h"
#include "statement.h"

namespace reaver
{
    namespace vapor
    {
        namespace parser { inline namespace _v1
        {
            struct expression;
            struct expression_list;
        }}

        namespace analyzer { inline namespace _v1
        {
            class scope;

            class expression : public statement
            {
            public:
                expression() = default;
                virtual ~expression() = default;

                expression(std::shared_ptr<variable> var) : _variable{ std::move(var) }
                {
                }

                std::shared_ptr<variable> get_variable() const
                {
                    if (!_variable)
                    {
                        assert(!"someone tried to get variable before analyzing... or forgot to set variable from analyze");
                    }

                    return _variable;
                }

                std::shared_ptr<type> get_type()
                {
                    if (!_variable)
                    {
                        assert(!"someone tried to get type before analyzing... or forgot to set variable from analyze");
                    }

                    return _variable->get_type();
                }

                future<std::shared_ptr<expression>> simplify_expr(optimization_context & ctx)
                {
                    return ctx.get_future_or_init(this, [&]() {
                        return make_ready_future().then([&, self = _shared_from_this()]{
                            return _simplify_expr(ctx);
                        });
                    });
                }

            protected:
                std::shared_ptr<expression> _shared_from_this()
                {
                    return std::static_pointer_cast<expression>(shared_from_this());
                }

                std::weak_ptr<expression> _weak_from_this()
                {
                    return std::static_pointer_cast<expression>(shared_from_this());
                }

                virtual future<std::shared_ptr<statement>> _simplify(optimization_context & ctx) override final
                {
                    return simplify_expr(ctx).then([&](auto && simplified) {
                        return static_cast<std::shared_ptr<statement>>(std::move(simplified));
                    });
                }

                virtual future<std::shared_ptr<expression>> _simplify_expr(optimization_context &) = 0;

                void _set_variable(std::shared_ptr<variable> var)
                {
                    assert(var);
                    _variable = var;
                }

            private:
                std::shared_ptr<variable> _variable;
            };

            class expression_list : public expression
            {
            private:
                virtual future<> _analyze() override;
                virtual future<std::shared_ptr<expression>> _simplify_expr(optimization_context &) override;

                virtual statement_ir _codegen_ir(ir_generation_context & ctx) const override
                {
                    return mbind(value, [&](auto && expr) {
                        return expr->codegen_ir(ctx);
                    });
                }

            public:
                virtual void print(std::ostream & os, std::size_t indent) const override;

                range_type range;
                std::vector<std::shared_ptr<expression>> value;
            };

            class variable_expression : public expression
            {
            public:
                variable_expression(std::shared_ptr<variable> var)
                {
                    _set_variable(std::move(var));
                }

                virtual void print(std::ostream & os, std::size_t indent) const override;

            private:
                virtual future<> _analyze() override
                {
                    return make_ready_future();
                }

                virtual future<std::shared_ptr<expression>> _simplify_expr(optimization_context &) override
                {
                    return make_ready_future(_shared_from_this());
                }

                virtual statement_ir _codegen_ir(ir_generation_context & ctx) const override
                {
                    return { codegen::ir::instruction {
                        none, none,
                        { boost::typeindex::type_id<codegen::ir::pass_value_instruction>() },
                        {},
                        codegen::ir::value{ get<0>(get_variable()->codegen_ir(ctx).back()) }
                    } };
                }
            };

            inline std::shared_ptr<expression> make_variable_expression(std::shared_ptr<variable> var)
            {
                return std::make_shared<variable_expression>(std::move(var));
            }

            std::shared_ptr<expression> preanalyze_expression(const parser::expression & expr, const std::shared_ptr<scope> & lex_scope);
            std::shared_ptr<expression> preanalyze_expression(const parser::expression_list & expr, const std::shared_ptr<scope> & lex_scope);
        }}
    }
}

