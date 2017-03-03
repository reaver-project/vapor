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

#include "vapor/analyzer/semantic/overloads.h"
#include "vapor/analyzer/expressions/call.h"
#include "vapor/analyzer/expressions/expression.h"
#include "vapor/analyzer/function.h"
#include "vapor/analyzer/symbol.h"
#include "vapor/analyzer/variables/member.h"
#include "vapor/analyzer/variables/member_assignment.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    enum class overload_match
    {
        first_better,
        equal,
        second_better
    };

    overload_match compare_overloads(function * lhs, function * rhs)
    {
        assert(0);
        assert(lhs);
        assert(rhs);
        return overload_match::equal;
    }

    bool is_valid(function * overload, const std::vector<expression *> & arguments, expression * base = nullptr)
    {
        auto param_begin = overload->parameters().begin();
        auto param_end = overload->parameters().end();
        auto arg_begin = arguments.begin();
        auto arg_end = arguments.end();

        if (overload->is_member())
        {
            if (!base)
            {
                logger::dlog() << overload->explain() << " not considered; is a member function, but the base expression is null";
                return false;
            }

            // TODO: conversions? possibly "interface inheritance" that operator. by Bjarne attempts (badly)?
            if ((*param_begin)->get_type() != base->get_type())
            {
                logger::dlog() << overload->explain() << " not considered; is a member function, but the base expression is of the wrong type";
                return false;
            }

            assert(param_begin != param_end);
            ++param_begin;
        }

        std::vector<variable *> provided_params;

        auto it = arg_begin;
        if ((it = std::find_if(it, arg_end, [](auto && arg) { return arg->get_variable()->is_member_assignment(); })) != arg_end)
        {
            auto arg_begin = it;
            auto possible_arg_end = it;

            if ((it = std::find_if(it, arg_end, [](auto && arg) { return !arg->get_variable()->is_member_assignment(); })) != arg_end)
            {
                assert(!"a non-mem-assignment argument after a mem-assignment argument");
            }

            // this is kind of a hack to allow passing member assignments that are *before the first matching one*
            // to pack arguments
            //
            // this will need a rework
            //
            // basically this is necessary to get the generic constructor to work
            // and I'm wondering wether this magic shouldn't just be limited to that single thing
            //
            // but I guess it'll be generally allowed to allow making forwarding wrappers that accept member assignments
            bool succeeded_before = false;

            while (arg_begin != arguments.end())
            {
                auto arg_type = dynamic_cast<member_assignment_type *>((*arg_begin)->get_type());
                assert(arg_type);
                auto arg = arg_type->get_associated_variable();

                auto it = param_begin;
                if ((it = std::find_if(param_begin,
                         param_end,
                         [&](auto && param) {
                             auto member_param = dynamic_cast<member_variable *>(param);
                             if (!member_param)
                             {
                                 return false;
                             }
                             return member_param->get_name() == arg->member_name();
                         }))
                    != param_end)
                {
                    assert((*it)->get_type()->matches(arg->get_assigned_type()));
                    succeeded_before = true;

                    provided_params.push_back(*it);

                    ++arg_begin;
                    continue;
                }

                if (succeeded_before)
                {
                    logger::dlog() << overload->explain() << " not considered; mismatch in member assignment arguments; ." << utf8(arg->member_name())
                                   << " did not match any members";
                    return false;
                }

                ++arg_begin;
            }

            if (succeeded_before)
            {
                arg_end = possible_arg_end;
            }
        }

        std::vector<type *> matching_space;
        matching_space.reserve(arg_end - arg_begin);

        // TODO: this will need some help in the future
        // in particular to be able to handle cases like
        // with (Ts... : type)
        // function foo(ts : Ts.uref..., last : int);
        // I know this can be done, but is more complex and we don't need it right now
        // and we can probably live with it not working until there's actually syntax for the above in the language
        // (i.e. when I'll be adding packs to the language for more than just the generic ctor)
        while (arg_begin != arg_end && param_begin != param_end)
        {
            if (std::find(provided_params.begin(), provided_params.end(), *param_begin) != provided_params.end())
            {
                // should this be a hard error? probably not
                assert(0);
                return false;
            }

            matching_space.clear();

            auto && param_type = (*param_begin)->get_type();

            if (!param_type->matches((*arg_begin)->get_type()))
            {
                if (!param_type->matches(matching_space))
                {
                    logger::dlog() << overload->explain() << " not considered; argument #" << arg_begin - arguments.begin() << " does not match the parameter #"
                                   << param_begin - overload->parameters().begin();
                    return false;
                }

                ++param_begin;
                continue;
            }

            bool last_matched;

            do
            {
                matching_space.push_back((*arg_begin)->get_type());
            } while ((last_matched = param_type->matches(matching_space)) && ++arg_begin != arg_end);

            if (matching_space.size() == 1 && !last_matched)
            {
                ++arg_begin;
            }

            ++param_begin;
        }

        assert(arg_begin == arg_end);

        auto ret = std::all_of(param_begin, param_end, [&](auto && param) {
            return std::find(provided_params.begin(), provided_params.end(), param) != provided_params.end() || param->get_default_value();
        });

        if (!ret)
        {
            logger::dlog() << overload->explain() << " not considered; some not provided parameters do not have a default value";
        }

        return ret;
    }

    // TODO FIXME: this is horrendeously inefficient
    // making these var_ref_expressions really needs to happen in select_overload_impl
    // but I don't know how to do that elegantly
    bool is_valid(function * overload, const std::vector<variable *> & arguments, variable * base = nullptr)
    {
        auto arg_exprs = fmap(arguments, [](auto && arg) { return make_variable_ref_expression(arg); });
        auto base_expr = base ? make_variable_ref_expression(base) : nullptr;

        return is_valid(overload, fmap(arg_exprs, [](auto && expr) { return expr.get(); }), base_expr.get());
    }

    template<typename ArgumentPointer, typename MakeExpression>
    future<std::unique_ptr<expression>> select_overload_impl(analysis_context & ctx,
        const range_type & range,
        std::vector<ArgumentPointer> & arguments,
        std::vector<function *> possible_overloads,
        ArgumentPointer base,
        MakeExpression && expr_maker)
    {
        auto original = possible_overloads;

        assert(!possible_overloads.empty());

        possible_overloads.erase(
            std::remove_if(possible_overloads.begin(), possible_overloads.end(), [&](auto && overload) { return !is_valid(overload, arguments, base); }),
            possible_overloads.end());

        std::sort(possible_overloads.begin(), possible_overloads.end(), [](auto && lhs, auto && rhs) {
            return compare_overloads(lhs, rhs) == overload_match::first_better;
        });

        // we might have ambiguous overloads
        // so we might have multiple "best matches"
        std::vector<function *> best_matches;
        for (auto && overload : possible_overloads)
        {
            if (!best_matches.empty() && compare_overloads(overload, best_matches.back()) == overload_match::equal)
            {
                break;
            }

            best_matches.push_back(overload);
        }

        if (best_matches.empty())
        {
            logger::dlog(logger::error) << "no matching overload found for expression at " << range;
            logger::dlog(logger::info) << "originally possible overloads:";
            fmap(original, [](auto && overload) {
                logger::dlog() << overload->explain();
                return unit{};
            });
            logger::default_logger().sync();
            std::terminate();
        }

        else if (best_matches.size() != 1)
        {
            logger::dlog(logger::error) << "multiple matching overloads found for expression at " << range;
            logger::dlog(logger::info) << "possible overloads:";
            fmap(best_matches, [](auto && overload) {
                logger::dlog() << overload->explain();
                return unit{};
            });
            logger::default_logger().sync();
            std::terminate();
        }

        auto overload = best_matches.front();

        if (overload->is_member())
        {
            assert(base);
            arguments.insert(arguments.begin(), base);
        }

        auto ret = expr_maker(overload, std::move(arguments));
        return make_ready_future<std::unique_ptr<expression>>(std::move(ret));
    }

    future<std::unique_ptr<expression>> select_overload(analysis_context & ctx,
        const range_type & range,
        std::vector<expression *> arguments,
        std::vector<function *> possible_overloads,
        expression * base)
    {
        return select_overload_impl(
            ctx, range, arguments, possible_overloads, base, [](auto &&... args) { return make_call_expression(std::forward<decltype(args)>(args)...); });
    }

    future<std::unique_ptr<expression>> select_overload(analysis_context & ctx,
        const range_type & range,
        std::vector<variable *> arguments,
        std::vector<function *> possible_overloads,
        variable * base)
    {
        return select_overload_impl(
            ctx, range, arguments, possible_overloads, base, [](auto &&... args) { return make_call_expression(std::forward<decltype(args)>(args)...); });
    }

    future<std::unique_ptr<expression>> resolve_overload(analysis_context & ctx,
        const range_type & range,
        expression * lhs,
        expression * rhs,
        lexer::token_type op)
    {
        return lhs->get_type()->get_candidates(op).then([&ctx, &range, lhs, rhs](auto && overloads) {
            assert(overloads.size());
            return select_overload(ctx, range, std::vector<expression *>{ lhs, rhs }, overloads);
        });
    }

    future<std::unique_ptr<expression>> resolve_overload(analysis_context & ctx,
        const range_type & range,
        expression * base_expr,
        lexer::token_type bracket_type,
        std::vector<expression *> arguments)
    {
        // mutable for `arguments`
        return base_expr->get_type()->get_candidates(bracket_type).then([&ctx, &range, arguments, base_expr](auto && overloads) mutable {
            assert(overloads.size());
            return select_overload(ctx, range, std::move(arguments), overloads, base_expr);
        });
    }
}
}
