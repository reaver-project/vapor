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

#include "vapor/analyzer/expressions/member_access.h"
#include "vapor/analyzer/expressions/binary.h"
#include "vapor/analyzer/expressions/member_assignment.h"
#include "vapor/analyzer/expressions/postfix.h"
#include "vapor/analyzer/symbol.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    future<> member_access_expression::_analyze(analysis_context & ctx)
    {
        auto & expr_ctx = get_context();
        assert(!expr_ctx.empty());

        auto last_postfix = std::find_if(expr_ctx.rbegin(), expr_ctx.rend(), [](auto && ctx_expr) { return ctx_expr.index() == 0; });
        assert(last_postfix != expr_ctx.rend());

        // this is actually conceptually "last_postfix + 1" (if last_postfix was a normal iterator)
        // reverse iterators are weird
        auto next = last_postfix.base();

        if (next != expr_ctx.end())
        {
            auto top_level = get<binary_expression *>(*next);

            // if this is on the LHS of a binary expression at the top level
            // of a postfix expression (in the future: also tuple-expression)
            // then this is an id part of a member assignment expression
            // and hence we need to do a Special Thing
            if (top_level->get_lhs() == this && top_level->get_operator() == lexer::token_type::assign)
            {
                _assignment_expr = make_member_assignment_expression(_name);
                _set_type(_assignment_expr->get_type());
                return make_ready_future();
            }
        }

        return get<postfix_expression *>(*last_postfix)->get_base_expression(ctx).then([&](auto && base) { this->set_base_expression(base); });
    }

    void member_access_expression::set_base_expression(expression * base)
    {
        _base = base;

        auto referenced_type = base->get_type()->get_member_type(_name);
        assert(referenced_type && "no member found for whatever reason");

        _referenced = base->get_member(_name);
        this->_set_type(referenced_type);
    }
}
}
