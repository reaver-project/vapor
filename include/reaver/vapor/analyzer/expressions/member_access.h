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

#pragma once

#include "expression.h"
#include "expression_ref.h"
#include "member_assignment.h"

namespace reaver::vapor::parser
{
inline namespace _v1
{
    struct member_expression;
}
}

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    class member_access_expression : public expression
    {
    public:
        member_access_expression(std::u32string name, type * referenced_type) : expression{ referenced_type }, _name{ std::move(name) }
        {
            assert(referenced_type);
        }

        member_access_expression(const parser::member_expression & parse);

        virtual void print(std::ostream & os, print_context) const override;

        const auto & parse() const
        {
            return _parse.get();
        }

        auto get_name() const
        {
            return _name;
        }

        virtual bool is_member_access() const override
        {
            return true;
        }

    private:
        member_access_expression(optional<synthesized_node<void>> parse, expression * referenced) : _parse{ parse }, _referenced{ referenced }
        {
        }

        virtual future<> _analyze(analysis_context &) override;

        virtual std::unique_ptr<expression> _clone_expr_with_replacement(replacements & repl) const override
        {
            if (_referenced)
            {
                return make_expression_ref(repl.get_replacement(_referenced));
            }

            if (!_base)
            {
                return std::unique_ptr<member_access_expression>{ new member_access_expression{ _name, get_type() } };
            }

            auto replaced_base = repl.try_get_replacement(_base);
            if (auto repl = replaced_base->get_member(_name))
            {
                return make_expression_ref(repl);
            }

            assert(!_assignment_expr);

            std::unique_ptr<member_access_expression> ret{ new member_access_expression{ _parse, nullptr } };
            ret->_base = replaced_base;
            ret->_set_type(get_type());
            return ret;
        }

        virtual future<expression *> _simplify_expr(recursive_context) override
        {
            return make_ready_future<expression *>(this);
        }

        virtual statement_ir _codegen_ir(ir_generation_context &) const override;
        virtual bool _invalidate_ir(ir_generation_context &) const override;

        optional<synthesized_node<void>> _parse;
        std::u32string _name;

        expression * _referenced = nullptr;
        mutable const expression * _base = nullptr;

        std::unique_ptr<member_assignment_expression> _assignment_expr;
    };

    inline std::unique_ptr<member_access_expression> preanalyze_member_access_expression(const parser::member_expression & parse, scope *)
    {
        return std::make_unique<member_access_expression>(parse);
    }

    inline std::unique_ptr<member_access_expression> make_member_access_expression(std::u32string name, type * ref_type)
    {
        return std::make_unique<member_access_expression>(std::move(name), ref_type);
    }
}
}
