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

#include "vapor/parser/expression_list.h"
#include "vapor/parser/lambda_expression.h"
#include "vapor/analyzer/function.h"
#include "vapor/analyzer/block.h"
#include "vapor/analyzer/return.h"
#include "vapor/analyzer/symbol.h"

reaver::future<> reaver::vapor::analyzer::_v1::function::simplify(reaver::vapor::analyzer::_v1::optimization_context & ctx)
{
    if (_body)
    {
        return _body->simplify(ctx)
            .then([&](auto && simplified) {
                _body = dynamic_cast<block *>(simplified);
            });
    }

    return make_ready_future();
}

reaver::future<reaver::vapor::analyzer::_v1::expression *> reaver::vapor::analyzer::_v1::function::simplify(reaver::vapor::analyzer::_v1::optimization_context & ctx, std::vector<variable *> args)
{
    if (_body)
    {
        return _body->simplify(ctx)
            .then([&](auto && simplified) {
                _body = dynamic_cast<block *>(simplified);
                assert(_body);

                return [&]{
                    if (args.size() && std::all_of(args.begin(), args.end(), [](auto && arg){ return arg->is_constant(); }))
                    {
                        auto body = _body->clone_with_replacement(
                            _arguments,
                            args
                        );

                        auto simplify = [ctx = std::make_shared<optimization_context>()](auto self, auto body)
                        {
                            return body->simplify(*ctx)
                                .then([&, old_body = body, ctx, self](auto && body) -> future<block *> {
                                    if (!ctx->did_something_happen())
                                    {
                                        return make_ready_future<block *>(dynamic_cast<block *>(body));
                                    }

                                    // ugh
                                    // but I don't know how else to write this
                                    // without creating a long overload for optctx
                                    ctx->~optimization_context();
                                    new (&*ctx) optimization_context();
                                    return self(self, body);
                                });
                        };

                        // this is a leak
                        return simplify(simplify, body.release());
                    }

                    return make_ready_future(_body);
                }().then([&](auto && body) {
                    auto returns = body->get_returns();

                    assert(body->has_return_expression() || returns.size());
                    auto var = body->has_return_expression() ? body->get_return_expression()->get_variable() : returns.front()->get_returned_variable();
                    auto begin = body->has_return_expression() ? returns.begin() : returns.begin() + 1;

                    if (!var->is_constant())
                    {
                        return make_ready_future<expression *>(nullptr);
                    }

                    if (std::all_of(begin, returns.end(), [](auto && ret) { return ret->get_returned_variable()->is_constant(); })
                        && std::all_of(begin, returns.end(), [&](auto && ret) { return ret->get_returned_variable()->is_equal(var); }))
                    {
                        return make_ready_future(make_variable_ref_expression(var).release());
                    }

                    return make_ready_future<expression *>(nullptr);
                });
            });
    }

    if (_compile_time_eval)
    {
        return make_ready_future((*_compile_time_eval)(ctx, args));
    }

    assert(0);
}

