/**
 * Vapor Compiler Licence
 *
 * Copyright © 2017-2019 Michał "Griwes" Dominiak
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
#include <reaver/future.h>

#include "vapor/analyzer/expressions/call.h"
#include "vapor/analyzer/expressions/expression.h"
#include "vapor/analyzer/expressions/pack.h"
#include "vapor/analyzer/expressions/type.h"
#include "vapor/analyzer/semantic/symbol.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    future<> call_expression::_analyze(analysis_context & ctx)
    {
        return _function->run_analysis_hooks(ctx, this, _args).then([&]() {
            if (_replacement_expr)
            {
                return _replacement_expr->analyze(ctx).then([&] { this->_set_type(_replacement_expr->get_type()); });
            }

            return _function->get_return_type().then([&](expression * type_expr) {
                assert(type_expr);
                assert(type_expr->get_type() == builtin_types().type.get());

                if (!type_expr->is_constant())
                {
                    replacements repl;
                    std::vector<std::shared_ptr<expression>> var_space;
                    std::vector<std::shared_ptr<expression>> expr_space = fmap(_args, [&](auto && arg) {
                        std::shared_ptr<expression> cloned = repl.claim(arg);
                        return cloned;
                    });

                    auto arg_begin = _args.begin();
                    auto arg_end = _args.end();
                    auto param_begin = _function->parameters().begin();
                    auto param_end = _function->parameters().end();

                    std::vector<type *> matching_space;
                    std::vector<expression *> matching_expressions;
                    matching_space.reserve(arg_end - arg_begin);
                    matching_expressions.reserve(arg_end - arg_begin);

                    while (arg_begin != arg_end && param_begin != param_end)
                    {
                        matching_space.clear();
                        matching_expressions.clear();

                        auto && param_type = (*param_begin)->get_type();

                        if (!param_type->matches((*arg_begin)->get_type()))
                        {
                            if (!param_type->matches(matching_space))
                            {
                                assert(0);
                            }

                            auto empty_pack = make_pack_expression();
                            repl.add_replacement(*param_begin, empty_pack.get());
                            var_space.push_back(std::move(empty_pack));

                            ++param_begin;
                            continue;
                        }

                        bool last_matched;

                        do
                        {
                            matching_space.push_back((*arg_begin)->get_type());
                            matching_expressions.push_back(*arg_begin);
                        } while ((last_matched = param_type->matches(matching_space)) && ++arg_begin != arg_end);

                        if (matching_space.size() == 1 && !last_matched)
                        {
                            if (*param_begin == type_expr)
                            {
                                // this is the simple case: one of the arguments is literally returned
                                // AND the function actually provides this information the way it should
                                // this is a special case that is PRIMARILY here for the generic constructor
                                //
                                // the other way does work for it too, but... it's not pretty resource-wise
                                auto expr = matching_expressions.front();
                                assert(expr->get_type() == builtin_types().type.get());

                                auto type_expr = expr->as<type_expression>();
                                assert(type_expr);
                                assert(type_expr->get_value() != builtin_types().type.get());

                                this->_set_type(type_expr->get_value());

                                return make_ready_future();
                            }

                            var_space.push_back(repl.claim(matching_expressions.front()));

                            ++arg_begin;
                            ++param_begin;
                            continue;
                        }

                        auto pack = make_pack_expression(fmap(matching_expressions, [&](auto && var) { return repl.claim(var); }), (*param_begin)->get_type());
                        repl.add_replacement(*param_begin, pack.get());
                        var_space.push_back(std::move(pack));

                        ++param_begin;
                    }

                    assert(arg_begin == arg_end);
                    assert(param_begin == param_end);

                    _cloned_type_expr = repl.claim(type_expr);

                    return simplification_loop(ctx, _cloned_type_expr).then([this, var_space, expr_space](expression * type_expr) {
                        assert(type_expr);
                        assert(type_expr->get_type() == builtin_types().type.get());

                        auto expr = type_expr->as<type_expression>();
                        assert(expr);
                        assert(expr->get_value() != builtin_types().type.get());

                        this->_set_type(expr->get_value());
                    });
                }

                auto expr = type_expr->as<type_expression>();
                assert(expr->get_value() != builtin_types().type.get());

                this->_set_type(expr->get_value());

                return make_ready_future();
            });
        });
    }
}
}
