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

#include "../types/member_assignment.h"
#include "expression.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    class member_assignment_expression : public expression
    {
    public:
        member_assignment_expression(std::u32string member_name) : _type{ make_member_assignment_type(std::move(member_name), this) }
        {
            _set_type(_type.get());
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

        void set_rhs(expression * rhs)
        {
            assert(!_rhs);
            _rhs = rhs;
        }

        auto get_rhs() const
        {
            assert(_rhs);
            return _rhs;
        }

        virtual bool is_member_assignment() const override
        {
            return true;
        }

        virtual void print(std::ostream & os, print_context ctx) const override
        {
            os << ctx << "member assignment expression @ " << this << '\n';
            _type->print(os, ctx.make_branch(false));
            _rhs->print(os, ctx.make_branch(true));
        }

    private:
        virtual std::unique_ptr<expression> _clone_expr_with_replacement(replacements & repl) const override
        {
            auto ret = std::make_unique<member_assignment_expression>(_type->member_name());
            ret->_rhs = _rhs;

            auto it = repl.expressions.find(_rhs);
            if (it != repl.expressions.end())
            {
                ret->_rhs = it->second;
            }

            return ret;
        }

        virtual statement_ir _codegen_ir(ir_generation_context & ctx) const override
        {
            assert(0);
            // return _rhs->codegen_ir(ctx);
        }

        expression * _rhs = nullptr;
        std::unique_ptr<expression> _owned_expr;

        std::unique_ptr<member_assignment_type> _type;
    };

    inline auto make_member_assignment_expression(std::u32string member_name)
    {
        return std::make_unique<member_assignment_expression>(std::move(member_name));
    }
}
}
