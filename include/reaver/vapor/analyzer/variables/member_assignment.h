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

#pragma once

#include "../expressions/variable.h"
#include "../types/member_assignment.h"
#include "variable.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    class member_assignment_variable : public variable
    {
    public:
        member_assignment_variable(std::u32string member_name) : _type{ make_member_assignment_type(std::move(member_name), this) }
        {
        }

        virtual type * get_type() const override
        {
            if (!_rhs)
            {
                return _type.get();
            }

            return _type->assigned_type();
        }

        const std::u32string member_name() const
        {
            return _type->member_name();
        }

        type * get_assigned_type() const
        {
            assert(_rhs);
            return _rhs->get_type();
        }

        void set_rhs(variable * rhs)
        {
            assert(!_rhs);
            _rhs = rhs;
            _rhs_expr = make_variable_ref_expression(_rhs);
        }

        auto get_rhs() const
        {
            assert(_rhs);
            return _rhs;
        }

        auto get_rhs_expression() const
        {
            assert(_rhs_expr);
            return _rhs_expr.get();
        }

    private:
        virtual std::unique_ptr<variable> _clone_with_replacement(replacements & repl) const override
        {
            auto ret = std::make_unique<member_assignment_variable>(_type->member_name());
            ret->_rhs = _rhs;

            auto it = repl.variables.find(_rhs);
            if (it != repl.variables.end())
            {
                ret->_rhs = it->second;
            }

            return ret;
        }

        virtual variable_ir _codegen_ir(ir_generation_context &) const override
        {
            assert(!"attempted to codegen a member-assignment-variable");
        }

        variable * _rhs = nullptr;
        std::unique_ptr<expression> _rhs_expr;

        std::unique_ptr<member_assignment_type> _type;
    };

    inline auto make_member_assignment_variable(std::u32string member_name)
    {
        return std::make_unique<member_assignment_variable>(std::move(member_name));
    }
}
}
