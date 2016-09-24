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

#include <string>

#include <boost/type_index.hpp>

#include <reaver/optional.h>

#include "variable.h"

namespace reaver
{
    namespace vapor
    {
        namespace codegen { inline namespace _v1
        {
            namespace ir
            {
                class instruction_type
                {
                    boost::typeindex::type_index _type;

                public:
                    instruction_type(boost::typeindex::type_index ti) : _type{ std::move(ti) }
                    {
                    }

                    bool operator==(const instruction_type & other) const
                    {
                        return _type == other._type;
                    }

                    template<typename T>
                    bool is() const
                    {
                        return _type == boost::typeindex::type_id<T>();
                    }

                    auto id() const
                    {
                        return _type;
                    }

                    std::string explain() const
                    {
                        return _type.pretty_name();
                    }
                };

                struct instruction
                {
                    optional<std::u32string> label;
                    optional<std::shared_ptr<variable>> declared_variable;

                    instruction_type instruction;
                    std::vector<value> operands;
                    value result;
                };

                std::ostream & operator<<(std::ostream & os, const instruction & inst);

                struct declaration_instruction {};
                struct function_call_instruction {};
                struct materialization_instruction {};
                struct pass_value_instruction {};
                struct return_instruction {};
                struct phi_instruction {};
            };
        }}
    }
}

