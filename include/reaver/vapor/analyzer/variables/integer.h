/**
 * Vapor Compiler Licence
 *
 * Copyright © 2016 Michał "Griwes" Dominiak
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

#include "../../parser/literal.h"
#include "../variables/literal.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    class integer_constant : public literal
    {
    public:
        integer_constant(const parser::integer_literal & parse) : _value{ utf8(parse.value.string) }
        {
        }

        integer_constant(boost::multiprecision::cpp_int value) : _value{ std::move(value) }
        {
        }

        virtual type * get_type() const override
        {
            return builtin_types().integer.get();
        }

        auto get_value() const
        {
            return _value;
        }

        virtual bool is_constant() const override
        {
            return true;
        }

        virtual bool is_equal(const variable * other_var) const override
        {
            auto other = dynamic_cast<const integer_constant *>(other_var);
            if (!other)
            {
                // todo: conversions somehow
                return false;
            }

            return other->get_value() == _value;
        }

    private:
        virtual std::unique_ptr<variable> _clone_with_replacement(replacements &) const override
        {
            return std::make_unique<integer_constant>(_value);
        }

        virtual variable_ir _codegen_ir(ir_generation_context &) const override;

        boost::multiprecision::cpp_int _value;
    };
}
}
