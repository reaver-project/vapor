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

#include <memory>

#include <boost/multiprecision/integer.hpp>

#include "../types/sized_integer.h"
#include "expression.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    class sized_integer_constant : public expression
    {
    public:
        sized_integer_constant(sized_integer * type, boost::multiprecision::cpp_int value) : expression{ type }, _value{ std::move(value) }, _type{ type }
        {
        }

        auto get_value() const
        {
            return _value;
        }

        virtual bool is_constant() const override
        {
            return true;
        }

        virtual void print(std::ostream &, print_context) const override
        {
            assert(0);
        }

    private:
        virtual std::unique_ptr<expression> _clone_expr_with_replacement(replacements &) const override
        {
            return std::make_unique<sized_integer_constant>(_type, _value);
        }

        virtual statement_ir _codegen_ir(ir_generation_context &) const override
        {
            assert(0);
        }

        virtual bool _is_equal(const expression * rhs) const override
        {
            auto other = rhs->as<sized_integer_constant>();
            if (!other)
            {
                // todo: conversions somehow
                return false;
            }

            return other->get_value() == _value;
        }

        boost::multiprecision::cpp_int _value;
        sized_integer * _type;
    };
}
}
