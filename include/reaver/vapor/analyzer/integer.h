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

#include <boost/multiprecision/integer.hpp>

#include "type.h"
#include "literal.h"
#include "../parser/literal.h"

namespace reaver
{
    namespace vapor
    {
        namespace analyzer { inline namespace _v1
        {
            class integer_type : public type
            {
            private:
            };

            class integer_constant : public literal
            {
            public:
                integer_constant(const parser::integer_literal & parse) : _value{ utf8(parse.value.string) }
                {
                }

                virtual std::shared_ptr<type> get_type() const override
                {
                    return builtin_types().integer;
                }

            private:
                boost::multiprecision::cpp_int _value;
            };

            inline std::shared_ptr<type> make_integer_type()
            {
                static auto int_t = std::make_shared<integer_type>();
                return int_t;
            }
        }}
    }
}

