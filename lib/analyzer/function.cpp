/**
 * Vapor Compiler Licence
 *
 * Copyright © 2016-2018 Michał "Griwes" Dominiak
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

#include "vapor/analyzer/function.h"
#include "vapor/analyzer/statements/block.h"
#include "vapor/analyzer/statements/return.h"
#include "vapor/analyzer/symbol.h"
#include "vapor/parser/expr.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    future<> function::simplify(recursive_context ctx)
    {
        if (_body)
        {
            return _body->simplify(ctx).then([&](auto && simplified) { _body = dynamic_cast<block *>(simplified); });
        }

        return make_ready_future();
    }

    future<expression *> function::simplify(recursive_context ctx, std::vector<expression *> arguments)
    {
        if (_body)
        {
            auto new_frame = call_frame{ this, arguments };

            if (auto expr = ctx.proper.results.get_call_result(new_frame))
            {
                return make_ready_future(expr.release());
            }

            if (std::find_if(ctx.call_stack.begin(), ctx.call_stack.end(), [&](auto && frame) { return frame == new_frame; }) != ctx.call_stack.end())
            {
                return make_ready_future<expression *>(nullptr);
            }

            return [&] {
                if (arguments.size())
                {
                    auto body = _body->clone_with_replacement(_parameters, arguments);
                    auto proper_ctx = std::make_shared<simplification_context>(ctx.proper.results);

                    auto simplify = [this, arguments, proper_ctx, ctx = recursive_context{ *proper_ctx, ctx.call_stack }](auto self, auto body)
                    {
                        auto new_ctx = ctx;
                        new_ctx.call_stack.push_back({ this, arguments });
                        return body->simplify(new_ctx).then([proper_ctx, old_body = body, new_ctx, self](auto && body)->future<statement *> {
                            if (!new_ctx.proper.did_something_happen())
                            {
                                return make_ready_future(body);
                            }

                            // ugh
                            // but I don't know how else to write this
                            // without creating a long overload for optctx
                            auto & res = proper_ctx->results;
                            proper_ctx->~simplification_context();
                            new (&*proper_ctx) simplification_context(res);
                            return self(self, body);
                        });
                    };

                    // this is a leak
                    return simplify(simplify, body.release());
                }

                assert(_body);
                return make_ready_future<statement *>(_body);
            }()
                       .then([=](auto && body) {
                           auto returns = body->get_returns();

                           auto body_block = dynamic_cast<block *>(body);
                           auto has_return_expr = body_block && body_block->has_return_expression();
                           assert(has_return_expr || returns.size());

                           auto expr = has_return_expr ? body_block->get_return_expression() : returns.front()->get_returned_expression();
                           auto begin = has_return_expr ? returns.begin() : returns.begin() + 1;

                           if (!expr->is_constant())
                           {
                               return make_ready_future<expression *>(nullptr);
                           }

                           if (std::all_of(begin, returns.end(), [](auto && ret) { return ret->get_returned_expression()->is_constant(); })
                               && std::all_of(begin, returns.end(), [&](auto && ret) { return ret->get_returned_expression()->is_equal(expr); }))
                           {
                               replacements a, b;
                               ctx.proper.results.save_call_result(call_frame{ this, arguments }, a.claim(expr));
                               return make_ready_future(b.claim(expr).release());
                           }

                           return make_ready_future<expression *>(nullptr);
                       });
        }

        if (_compile_time_eval)
        {
            return (*_compile_time_eval)(ctx, arguments);
        }

        return make_ready_future<expression *>(nullptr);
    }

    future<> function::run_analysis_hooks(analysis_context & ctx, call_expression * expr, std::vector<expression *> args)
    {
        return foldl(_analysis_hooks, make_ready_future(), [&ctx, expr, args](auto && prev, auto && hook) {
            return prev.then([&hook, &ctx, expr, args] { return hook(ctx, expr, args); });
        });
    }

    void function::print(std::ostream & os, print_context ctx) const
    {
        os << styles::def << ctx << styles::rule_name << "function";
        os << styles::def << " @ " << styles::address << this << styles::def;

        if (_range)
        {
            os << " (" << styles::range << _range.value() << styles::def << ')';
        }

        os << ": " << _explanation << '\n';
    }

    codegen::ir::function function::codegen_ir(ir_generation_context & ctx) const
    {
        auto state = ctx.top_level_generation;
        ctx.top_level_generation = false;

        if (!_ir)
        {
            assert(_codegen);
            _ir = _codegen.value()(ctx);
            _ir->is_member = _is_member;
            _ir->is_builtin = _is_builtin;

            if (_entry)
            {
                _ir->is_entry = true;
                _ir->entry_variable = _entry_expr->codegen_ir(ctx).back().result;
            }
        }

        if (state)
        {
            ctx.add_generated_function(this);
        }

        ctx.top_level_generation = state;

        return *_ir;
    }
}
}
