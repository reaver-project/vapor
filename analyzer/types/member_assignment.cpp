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
#include "vapor/analyzer/expressions/variable.h"
#include "vapor/analyzer/symbol.h"
#include "vapor/analyzer/types/unconstrained.h"
#include "vapor/analyzer/variables/member_assignment.h"
#include "vapor/analyzer/variables/type.h"

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

        auto var = make_blank_variable(builtin_types().unconstrained.get());

        auto overload = make_function("member assignment", nullptr, { _var, var.get() }, [](auto &&) -> codegen::ir::function {
            assert(!"trying to codegen a member-assignment expression");
        });
        overload->set_return_type(assigned_type()->get_expression());
        overload->add_analysis_hook([this](std::vector<variable *> args) {
            assert(args.size() == 2);
            assert(args.front() == _var);
            _var->set_rhs(args.back());
        });
        overload->set_eval([this](auto &&...) { return make_ready_future(make_variable_ref_expression(_var).release()); });

        auto ret = overload.get();

        {
            // TODO: this is useless, move the generation of the overload to the constructor
            std::lock_guard<std::mutex> lock{ _storage_lock };
            _fun_storage.push_back(std::move(overload));
            _var_storage.push_back(std::move(var));
        }

        return make_ready_future(std::vector<function *>{ ret });
    }
}
}
