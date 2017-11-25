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

#include "../types/type.h"
#include "expression.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    class type_expression : public expression
    {
    public:
        type_expression(type * t) : expression{ builtin_types().type.get() }, _type{ t }
        {
        }

        type * get_value() const
        {
            return _type;
        }

        virtual bool is_constant() const override
        {
            return _type;
        }

        virtual void print(std::ostream & os, print_context ctx) const override
        {
            os << styles::def << ctx << styles::rule_name << "type-expression";
            print_address_range(os, this);
            os << '\n';

            auto type_ctx = ctx.make_branch(true);
            os << styles::def << type_ctx << styles::subrule_name << "value:\n";
            _type->print(os, type_ctx.make_branch(true));
        }

        virtual declaration_ir declaration_codegen_ir(ir_generation_context & ctx) const override
        {
            return { { std::get<std::shared_ptr<codegen::ir::variable>>(_codegen_ir(ctx).back().result) } };
        }

    private:
        virtual std::unique_ptr<expression> _clone_expr_with_replacement(replacements &) const override
        {
            return std::make_unique<type_expression>(_type);
        }

        virtual statement_ir _codegen_ir(ir_generation_context & ctx) const override
        {
            auto ret = codegen::ir::make_variable(codegen::ir::builtin_types().type);
            ret->refers_to = _type->codegen_type(ctx);

            return { codegen::ir::instruction{
                std::nullopt, std::nullopt, { boost::typeindex::type_id<codegen::ir::pass_value_instruction>() }, {}, std::move(ret) } };
        }

        virtual bool _is_equal(const expression * rhs) const override
        {
            auto type_rhs = rhs->as<type_expression>();
            return type_rhs && _type == type_rhs->_type;
        }

        type * _type;
    };

    inline auto make_type_expression(type * t)
    {
        return std::make_unique<type_expression>(t);
    }
}
}
