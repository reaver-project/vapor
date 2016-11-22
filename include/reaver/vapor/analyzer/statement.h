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

#include <reaver/future.h>

#include "../codegen/ir/variable.h"
#include "../codegen/ir/instruction.h"
#include "ir_context.h"
#include "analysis_context.h"
#include "simplification_context.h"

namespace reaver
{
    namespace vapor
    {
        namespace parser { inline namespace _v1
        {
            struct statement;
        }}

        namespace analyzer { inline namespace _v1
        {
            struct replacements
            {
                std::unordered_map<variable const *, variable *> variables;
                std::unordered_map<statement const *, statement *> statements = {};
                std::unordered_map<expression const *, expression *> expressions = {};
            };

            class return_statement;
            class scope;

            using statement_ir = std::vector<codegen::ir::instruction>;

            class statement
            {
            public:
                statement() = default;
                virtual ~statement() = default;

                future<> analyze(analysis_context & ctx)
                {
                    if (!_is_future_assigned)
                    {
                        std::lock_guard<std::mutex> lock{ _future_lock };
                        if (!_is_future_assigned)
                        {
                            _analysis_future = _analyze(ctx);
                            _analysis_future->on_error([](std::exception_ptr ptr){
                                try
                                {
                                    std::rethrow_exception(ptr);
                                }

                                catch (...)
                                {
                                    std::terminate();
                                }
                            }).detach(); // this is wrooong
                            _is_future_assigned = true;
                        }
                    }

                    return *_analysis_future;
                }

                std::unique_ptr<statement> clone_with_replacement(replacements & repl) const
                {
                    auto ret = _clone_with_replacement(repl);
                    repl.statements[this] = ret.get();
                    return ret;
                }

                std::unique_ptr<statement> clone_with_replacement(const std::vector<variable *> & to_replace, const std::vector<variable *> & replacements) const
                {
                    assert(to_replace.size() == replacements.size());
                    std::unordered_map<const variable *, variable *> replacement_map;
                    for (std::size_t i = 0; i < to_replace.size(); ++i)
                    {
                        replacement_map.emplace(to_replace[i], replacements[i]);
                    }

                    struct replacements repl{ std::move(replacement_map) };
                    return _clone_with_replacement(repl);
                }

                future<statement *> simplify(simplification_context & ctx)
                {
                    return ctx.get_future_or_init(this, [&]() {
                        return make_ready_future().then([this, &ctx]() {
                            return _simplify(ctx);
                        });
                    });
                }

                virtual std::vector<const return_statement *> get_returns() const
                {
                    return {};
                }

                virtual bool always_returns() const
                {
                    return false;
                }

                virtual void print(std::ostream &, std::size_t indent) const = 0;

                statement_ir codegen_ir(ir_generation_context & ctx) const
                {
                    if (!_ir)
                    {
                        _ir = _codegen_ir(ctx);
                    }

                    return *_ir;
                }

            private:
                virtual future<> _analyze(analysis_context &) = 0;
                virtual std::unique_ptr<statement> _clone_with_replacement(replacements &) const = 0;
                virtual future<statement *> _simplify(simplification_context &) = 0;
                virtual statement_ir _codegen_ir(ir_generation_context &) const = 0;

                std::mutex _future_lock;
                std::atomic<bool> _is_future_assigned{ false };
                optional<future<>> _analysis_future;
                mutable optional<statement_ir> _ir;
            };

            class null_statement : public statement
            {
            public:
                virtual void print(std::ostream &, std::size_t) const override
                {
                }

            private:
                virtual future<> _analyze(analysis_context &) override
                {
                    return make_ready_future();
                }

                virtual std::unique_ptr<statement> _clone_with_replacement(replacements &) const override
                {
                    return std::make_unique<null_statement>();
                }

                virtual future<statement *> _simplify(simplification_context &) override
                {
                    return make_ready_future<statement *>(this);
                }

                virtual statement_ir _codegen_ir(ir_generation_context &) const override
                {
                    return {};
                }
            };

            std::unique_ptr<statement> preanalyze_statement(const parser::statement & parse, scope *& lex_scope);

            inline std::unique_ptr<statement> make_null_statement()
            {
                return std::make_unique<null_statement>();
            }
        }}
    }
}

