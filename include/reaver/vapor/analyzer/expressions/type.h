/**
 * Vapor Compiler Licence
 *
 * Copyright © 2016-2019 Michał "Griwes" Dominiak
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
#include "constant.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    enum class type_kind
    {
        type
    };

    class type_expression : public constant
    {
    private:
        type * _select(type_kind kind)
        {
            switch (kind)
            {
                case type_kind::type:
                    return builtin_types().type.get();
            }
        }

    public:
        type_expression(type * t, type_kind kind = type_kind::type) : constant{ _select(kind) }, _type{ t }
        {
        }

        type_expression(type * t, type * base_type) : constant{ base_type }, _type{ t }
        {
        }

        type * get_value() const
        {
            return _type;
        }

        virtual void print(std::ostream & os, print_context ctx) const override;

    private:
        virtual std::unique_ptr<expression> _clone_expr(replacements &) const override;
        virtual constant_init_ir _constinit_ir(ir_generation_context & ctx) const override;

        virtual bool _is_equal(const expression * rhs) const override;

        virtual std::unique_ptr<google::protobuf::Message> _generate_interface() const override;

        type * _type;
    };

    inline auto make_type_expression(type * t)
    {
        return std::make_unique<type_expression>(t);
    }
}
}
