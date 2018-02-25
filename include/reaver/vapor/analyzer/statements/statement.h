/**
 * Vapor Compiler Licence
 *
 * Copyright © 2014, 2016-2018 Michał "Griwes" Dominiak
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

#include "../../codegen/ir/function.h"
#include "../../codegen/ir/instruction.h"
#include "../../codegen/ir/module.h"
#include "../../codegen/ir/variable.h"
#include "../../print_helpers.h"
#include "../ir_context.h"
#include "../semantic/context.h"
#include "../simplification/context.h"
#include "../simplification/replacements.h"

namespace reaver::vapor::parser
{
inline namespace _v1
{
    struct statement;
}
}

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    class return_statement;
    class scope;

    using statement_ir = std::vector<codegen::ir::instruction>;
    using declaration_ir = std::vector<std::variant<std::shared_ptr<codegen::ir::variable>, codegen::ir::function, codegen::ir::module>>;

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
                    _analysis_future
                        ->on_error([](std::exception_ptr ptr) {
                            try
                            {
                                std::rethrow_exception(ptr);
                            }

                            catch (...)
                            {
                                std::terminate();
                            }
                        })
                        .detach(); // this is wrooong
                    _is_future_assigned = true;
                }
            }

            return *_analysis_future;
        }

        friend class replacements;

    private:
        std::unique_ptr<statement> clone_with_replacement(replacements & repl) const
        {
            return _clone_with_replacement(repl);
        }

    public:
        std::unique_ptr<statement> clone_with_replacement(const std::vector<expression *> & to_replace, const std::vector<expression *> & replacements) const
        {
            assert(to_replace.size() == replacements.size());
            class replacements repl;
            for (std::size_t i = 0; i < to_replace.size(); ++i)
            {
                repl.add_replacement(to_replace[i], replacements[i]);
            }

            return _clone_with_replacement(repl);
        }

        future<statement *> simplify(recursive_context ctx)
        {
            return ctx.proper.get_future_or_init(this, [&]() { return make_ready_future().then([this, ctx]() { return _simplify(ctx); }); });
        }

        virtual std::vector<const return_statement *> get_returns() const
        {
            return {};
        }

        virtual bool always_returns() const
        {
            return false;
        }

        virtual void print(std::ostream &, print_context) const = 0;

        statement_ir codegen_ir(ir_generation_context & ctx) const
        {
            if (!_ir)
            {
                auto ir = _codegen_ir(ctx);

                if (!_ir)
                {
                    _ir = std::move(ir);
                }
            }

            auto ret = *_ir;
            if (_invalidate_ir(ctx))
            {
                _ir = std::nullopt;
            }
            return ret;
        }

        virtual declaration_ir declaration_codegen_ir(ir_generation_context &) const
        {
            return {};
        }

        std::optional<ast_node> get_ast_info() const
        {
            return _parse_info;
        }

    protected:
        void _set_ast_info(ast_node info)
        {
            assert(!_parse_info);
            _parse_info = info;
        }

    private:
        virtual future<> _analyze(analysis_context &)
        {
            return make_ready_future();
        }

        virtual std::unique_ptr<statement> _clone_with_replacement(replacements &) const = 0;

        virtual bool _invalidate_ir(ir_generation_context &) const
        {
            return false;
        }

        virtual future<statement *> _simplify(recursive_context)
        {
            return make_ready_future(this);
        }

        virtual statement_ir _codegen_ir(ir_generation_context &) const = 0;

        std::mutex _future_lock;
        std::atomic<bool> _is_future_assigned{ false };
        std::optional<future<>> _analysis_future;
        mutable std::optional<statement_ir> _ir;

        std::optional<ast_node> _parse_info;
    };

    class null_statement : public statement
    {
    public:
        virtual void print(std::ostream &, print_context) const override
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

        virtual future<statement *> _simplify(recursive_context) override
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
}
}
