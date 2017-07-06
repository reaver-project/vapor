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

#include "vapor/analyzer/types/member_assignment.h"
#include "vapor/analyzer/expressions/blank.h"
#include "vapor/analyzer/expressions/call.h"
#include "vapor/analyzer/expressions/expression_ref.h"
#include "vapor/analyzer/expressions/member_assignment.h"
#include "vapor/analyzer/expressions/type.h"
#include "vapor/analyzer/symbol.h"
#include "vapor/analyzer/types/unconstrained.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    member_assignment_type::~member_assignment_type() = default;

    future<std::vector<function *>> member_assignment_type::get_candidates(lexer::token_type tt) const
    {
        assert(!_assigned);

        if (tt != lexer::token_type::assign)
        {
            assert(0);
            return make_ready_future(std::vector<function *>{});
        }

        auto expr = make_blank_expression(builtin_types().unconstrained.get());

        auto overload = make_function("member assignment", nullptr, { _expr, expr.get() }, [](auto &&) -> codegen::ir::function {
            assert(!"trying to codegen a member-assignment expression");
        });
        overload->set_return_type(assigned_type()->get_expression());
        overload->add_analysis_hook([this](auto &&, auto && call_expr, std::vector<expression *> args) {
            assert(args.size() == 2);
            assert(args.front()->_get_replacement() == _expr);
            _expr->set_rhs(args.back());
            call_expr->replace_with(make_expression_ref(_expr));

            return make_ready_future();
        });
        overload->set_eval([this](auto &&...) -> future<expression *> {
            assert(0); // need to pass this in a way that conveys the information that this isn't the owner of it
            // (or a way to make it the owner of it, which I guess should be doable since this is eval)
            // return make_ready_future(make_variable_ref_expression(_var).release());
        });

        auto ret = overload.get();

        {
            // TODO: this is useless, move the generation of the overload to the constructor
            std::lock_guard<std::mutex> lock{ _storage_lock };
            _fun_storage.push_back(std::move(overload));
            _expr_storage.push_back(std::move(expr));
        }

        return make_ready_future(std::vector<function *>{ ret });
    }
}
}
