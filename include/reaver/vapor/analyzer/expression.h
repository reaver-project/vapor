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
#include "helpers.h"

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

                expression(std::unique_ptr<variable> var) : _variable{ std::move(var) }
                {
                }

                // do NOT abuse the fact this is virtual!
                virtual variable * get_variable() const
                {
                    if (!_variable)
                    {
                        assert(!"someone tried to get variable before analyzing... or forgot to set variable from analyze");
                    }

                    return _variable.get();
                }

                type * get_type()
                {
                    auto var = get_variable();

                    if (!var)
                    {
                        assert(!"someone tried to get type before analyzing... or forgot to set variable from analyze");
                    }

                    return var->get_type();
                }

                future<expression *> simplify_expr(optimization_context & ctx)
                {
                    return ctx.get_future_or_init(this, [&]() {
                        return make_ready_future().then([this, &ctx]() {
                            return _simplify_expr(ctx);
                        });
                    });
                }

            protected:
                virtual future<statement *> _simplify(optimization_context & ctx) override final
                {
                    return simplify_expr(ctx).then([&](auto && simplified) -> statement * {
                        return simplified;
                    });
                }

                virtual future<expression *> _simplify_expr(optimization_context &) = 0;

                void _set_variable(std::unique_ptr<variable> var)
                {
                    assert(var);
                    assert(!_variable);
                    _variable = std::move(var);
                }

                void _set_variable(variable * ptr, optimization_context & ctx)
                {
                    replace_uptr(_variable, ptr, ctx);
                }

            private:
                std::unique_ptr<variable> _variable;
            };

            class expression_list : public expression
            {
            private:
                virtual future<> _analyze() override;
                virtual future<expression *> _simplify_expr(optimization_context &) override;

                virtual statement_ir _codegen_ir(ir_generation_context & ctx) const override
                {
                    return mbind(value, [&](auto && expr) {
                        return expr->codegen_ir(ctx);
                    });
                }

            public:
                virtual void print(std::ostream & os, std::size_t indent) const override;

                range_type range;
                std::vector<std::unique_ptr<expression>> value;
            };

            class variable_expression : public expression
            {
            public:
                variable_expression(std::unique_ptr<variable> var)
                {
                    _set_variable(std::move(var));
                }

                virtual void print(std::ostream & os, std::size_t indent) const override;

            private:
                virtual future<> _analyze() override
                {
                    return make_ready_future();
                }

                virtual future<expression *> _simplify_expr(optimization_context & ctx) override
                {
                    return get_variable()->simplify(ctx)
                        .then([&](auto && simplified) -> expression * {
                            this->_set_variable(simplified, ctx);
                            return this;
                        });
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

            inline std::unique_ptr<expression> make_variable_expression(std::unique_ptr<variable> var)
            {
                return std::make_unique<variable_expression>(std::move(var));
            }

            class variable_ref_expression : public expression
            {
            public:
                variable_ref_expression(variable * var) : _referenced(var)
                {
                }

                virtual variable * get_variable() const final override
                {
                    return _referenced;
                }

                virtual void print(std::ostream & os, std::size_t indent) const override;

            private:
                virtual future<> _analyze() override
                {
                    return make_ready_future();
                }

                virtual future<expression *> _simplify_expr(optimization_context & ctx) override
                {
                    return _referenced->simplify(ctx)
                        .then([&](auto && simplified) -> expression * {
                            _referenced = simplified;
                            return this;
                        });
                }

                virtual statement_ir _codegen_ir(ir_generation_context & ctx) const override
                {
                    return { codegen::ir::instruction {
                        none, none,
                        { boost::typeindex::type_id<codegen::ir::pass_value_instruction>() },
                        {},
                        codegen::ir::value{ get<0>(_referenced->codegen_ir(ctx).back()) }
                    } };
                }

                variable * _referenced;
            };

            inline std::unique_ptr<expression> make_variable_ref_expression(variable * var)
            {
                return std::make_unique<variable_ref_expression>(var);
            }

            std::unique_ptr<expression> preanalyze_expression(const parser::expression & expr, scope * lex_scope);
            std::unique_ptr<expression> preanalyze_expression(const parser::expression_list & expr, scope * lex_scope);
        }}
    }
}

