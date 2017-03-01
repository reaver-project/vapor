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
                return false;
            }

            // TODO: conversions? possibly "interface inheritance" that operator. by Bjarne attempts (badly)?
            if ((*param_begin)->get_type() != base->get_type())
            {
                return false;
            }

            assert(param_begin != param_end);
            ++param_begin;
        }

        auto it = arg_begin;
        if ((it = std::find_if(it, arg_end, [](auto && arg) { return arg->get_variable()->is_member_assignment(); })) != arg_end)
        {
            if ((it = std::find_if(it, arg_end, [](auto && arg) { return !arg->get_variable()->is_member_assignment(); })) != arg_end)
            {
                assert(!"a non-mem-assignment argument after a mem-assignment argument");
            }

            assert(0);
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
            matching_space.clear();

            auto && param_type = (*param_begin)->get_type();

            if (!param_type->matches((*arg_begin)->get_type()))
            {
                if (!param_type->matches(matching_space))
                {
                    assert(0);
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

        return std::all_of(param_begin, param_end, [](auto && param) { return param->get_default_value(); });
    }

    auto select_overload(analysis_context & ctx, std::vector<expression *> arguments, std::vector<function *> possible_overloads, expression * base = nullptr)
    {
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

        assert(best_matches.size() > 0);
        assert(best_matches.size() < 2);
        auto overload = best_matches.front();

        if (overload->is_member())
        {
            assert(base);
            arguments.insert(arguments.begin(), base);
        }

        auto ret = make_call_expression(overload, std::move(arguments));
        return make_ready_future<std::unique_ptr<expression>>(std::move(ret));
    }

    future<std::unique_ptr<expression>> resolve_overload(analysis_context & ctx, expression * lhs, expression * rhs, lexer::token_type op)
    {
        return lhs->get_type()->get_candidates(op).then([&ctx, lhs, rhs](auto && overloads) { return select_overload(ctx, { lhs, rhs }, overloads); });
    }

    future<std::unique_ptr<expression>> resolve_overload(analysis_context & ctx,
        expression * base_expr,
        lexer::token_type bracket_type,
        std::vector<expression *> arguments)
    {
        // mutable for `arguments`
        return base_expr->get_type()->get_candidates(bracket_type).then([&ctx, arguments, base_expr](auto && overloads) mutable {
            return select_overload(ctx, std::move(arguments), overloads, base_expr);
        });
    }
}
}
