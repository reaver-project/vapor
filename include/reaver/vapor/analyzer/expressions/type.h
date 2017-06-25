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

        virtual void print(std::ostream &, print_context) const override
        {
            assert(0);
        }

    private:
        virtual std::unique_ptr<expression> _clone_expr_with_replacement(replacements &) const override
        {
            return std::make_unique<type_expression>(_type);
        }

        virtual statement_ir _codegen_ir(ir_generation_context & ctx) const override
        {
            // auto ret = codegen::ir::make_variable(codegen::ir::builtin_types().type);
            // ret->refers_to = _type->codegen_type(ctx);
            // return ret;
            assert(0);
        }

        virtual bool _is_equal(const expression * rhs) const override
        {
            auto type_rhs = dynamic_cast<const type_expression *>(rhs);
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
