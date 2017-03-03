/**
 * Vapor Compiler Licence
 *
 * Copyright © 2017 Michał "Griwes" Dominiak
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
#include "vapor/analyzer/symbol.h"
#include "vapor/analyzer/variables/expression.h"
#include "vapor/analyzer/variables/pack.h"
#include "vapor/analyzer/variables/type.h"
#include "vapor/analyzer/variables/variable.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    future<> call_expression::_analyze(analysis_context & ctx)
    {
        return _function->run_analysis_hooks(ctx, this, fmap(_args, [](auto && arg) { return arg->get_variable(); })).then([&]() {
            if (_replacement_expr)
            {
                return _replacement_expr->analyze(ctx);
            }

            return _function->get_return_type().then([&](expression * type_expr) {
                assert(type_expr);
                auto var = type_expr->get_variable();
                assert(var->get_type() == builtin_types().type.get());

                if (!var->is_constant())
                {
                    replacements repl;
                    std::vector<std::shared_ptr<variable>> var_space;
                    std::vector<std::shared_ptr<expression>> expr_space = fmap(_args, [&](auto && arg) {
                        std::shared_ptr<expression> cloned = arg->clone_expr_with_replacement(repl);
                        repl.expressions[arg] = cloned.get();
                        return cloned;
                    });

                    auto arg_begin = _args.begin();
                    auto arg_end = _args.end();
                    auto param_begin = _function->parameters().begin();
                    auto param_end = _function->parameters().end();

                    std::vector<type *> matching_space;
                    std::vector<variable *> matching_variables;
                    matching_space.reserve(arg_end - arg_begin);
                    matching_variables.reserve(arg_end - arg_begin);

                    while (arg_begin != arg_end && param_begin != param_end)
                    {
                        matching_space.clear();
                        matching_variables.clear();

                        auto && param_type = (*param_begin)->get_type();

                        if (!param_type->matches((*arg_begin)->get_type()))
                        {
                            if (!param_type->matches(matching_space))
                            {
                                assert(0);
                            }

                            auto empty_pack = make_pack_variable();
                            repl.variables[*param_begin] = empty_pack.get();
                            var_space.push_back(std::move(empty_pack));

                            ++param_begin;
                            continue;
                        }

                        bool last_matched;

                        do
                        {
                            matching_space.push_back((*arg_begin)->get_type());
                            matching_variables.push_back((*arg_begin)->get_variable());
                        } while ((last_matched = param_type->matches(matching_space)) && ++arg_begin != arg_end);

                        if (matching_space.size() == 1 && !last_matched)
                        {
                            if (*param_begin == var)
                            {
                                // this is the simple case: one of the arguments is literally returned
                                // AND the function actually provides this information the way it should
                                // this is a special case that is PRIMARILY here for the generic constructor
                                //
                                // the other way does work for it too, but... it's not pretty resource-wise
                                auto var = matching_variables.front();
                                assert(var->get_type() == builtin_types().type.get());

                                auto type_var = dynamic_cast<type_variable *>(var);
                                assert(type_var);
                                assert(type_var->get_value() != builtin_types().type.get());

                                _var = make_expression_ref_variable(this, type_var->get_value());

                                return make_ready_future();
                            }

                            auto cloned = matching_variables.front()->clone_with_replacement(repl);
                            repl.variables[*param_begin] = cloned.get();
                            var_space.push_back(std::move(cloned));

                            ++arg_begin;
                            ++param_begin;
                            continue;
                        }

                        auto range = &*_range;
                        (void)range;
                        auto pack = make_pack_variable(
                            fmap(matching_variables, [&](auto && var) { return var->clone_with_replacement(repl); }), (*param_begin)->get_type());
                        repl.variables[*param_begin] = pack.get();
                        var_space.push_back(std::move(pack));

                        ++param_begin;
                    }

                    assert(arg_begin == arg_end);
                    assert(param_begin == param_end);

                    _cloned_type_expr = type_expr->clone_expr_with_replacement(repl);

                    auto cont = [this, ctx = std::make_shared<simplification_context>()](auto self)->future<expression *>
                    {
                        return _cloned_type_expr->simplify_expr(*ctx).then([this, ctx, self](auto && simpl) -> future<expression *> {
                            replace_uptr(_cloned_type_expr, simpl, *ctx);

                            if (_cloned_type_expr->get_variable()->is_constant() || !ctx->did_something_happen())
                            {
                                return make_ready_future<expression *>(_cloned_type_expr.get());
                            }

                            ctx->~simplification_context();
                            new (&*ctx) simplification_context();
                            return self(self);
                        });
                    };

                    return cont(cont).then([this, var_space, expr_space](auto && type_expr) {
                        assert(type_expr);
                        auto var = type_expr->get_variable();
                        assert(var->get_type() == builtin_types().type.get());

                        auto type_var = dynamic_cast<type_variable *>(var);
                        assert(type_var);
                        assert(type_var->get_value() != builtin_types().type.get());

                        _var = make_expression_ref_variable(this, type_var->get_value());
                    });
                }

                auto type_var = dynamic_cast<type_variable *>(var);
                assert(type_var->get_value() != builtin_types().type.get());

                _var = make_expression_ref_variable(this, type_var->get_value());

                return make_ready_future();
            });
        });
    }
}
}
