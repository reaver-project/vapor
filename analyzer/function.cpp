/**
 * Vapor Compiler Licence
 *
 * Copyright © 2016-2017 Michał "Griwes" Dominiak
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
#include "vapor/parser/expression_list.h"
#include "vapor/parser/lambda_expression.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    future<> function::simplify(simplification_context & ctx)
    {
        if (_body)
        {
            return _body->simplify(ctx).then([&](auto && simplified) { _body = dynamic_cast<block *>(simplified); });
        }

        return make_ready_future();
    }

    future<expression *> function::simplify(simplification_context & ctx, std::vector<expression *> arguments)
    {
        if (_body)
        {
            if (!std::any_of(arguments.begin(), arguments.end(), [](auto && arg) { return arg->is_constant(); }))
            {
                return make_ready_future<expression *>(nullptr);
            }

            return [&] {
                if (arguments.size())
                {
                    auto body = _body->clone_with_replacement(_parameters, arguments);

                    auto simplify = [ctx = std::make_shared<simplification_context>()](auto self, auto body)
                    {
                        return body->simplify(*ctx).then([old_body = body, ctx, self](auto && body)->future<block *> {
                            if (!ctx->did_something_happen())
                            {
                                return make_ready_future<block *>(dynamic_cast<block *>(body));
                            }

                            // ugh
                            // but I don't know how else to write this
                            // without creating a long overload for optctx
                            ctx->~simplification_context();
                            new (&*ctx) simplification_context();
                            return self(self, body);
                        });
                    };

                    // this is a leak
                    return simplify(simplify, body.release());
                }

                return make_ready_future(_body);
            }()
                       .then([&](auto && body) {
                           auto returns = body->get_returns();

                           assert(body->has_return_expression() || returns.size());
                           auto expr = body->has_return_expression() ? body->get_return_expression() : returns.front()->get_returned_expression();
                           auto begin = body->has_return_expression() ? returns.begin() : returns.begin() + 1;

                           if (!expr->is_constant())
                           {
                               return make_ready_future<expression *>(nullptr);
                           }

                           if (std::all_of(begin, returns.end(), [](auto && ret) { return ret->get_returned_expression()->is_constant(); })
                               && std::all_of(begin, returns.end(), [&](auto && ret) { return ret->get_returned_expression()->is_equal(expr); }))
                           {
                               return make_ready_future(expr);
                           }

                           return make_ready_future<expression *>(nullptr);
                       });
        }

        if (_compile_time_eval)
        {
            return (*_compile_time_eval)(ctx, arguments);
        }

        assert(0);
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
            os << " (" << styles::range << _range.get() << styles::def << ')';
        }

        os << ": " << _explanation << '\n';
    }
}
}
