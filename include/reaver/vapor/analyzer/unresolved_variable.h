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

#include "variable.h"
#include "type.h"
#include "expression.h"
#include "type_variable.h"

namespace reaver
{
    namespace vapor
    {
        namespace analyzer { inline namespace _v1
        {
            class unresolved_variable : public variable
            {
            public:
                unresolved_variable(std::u32string name) : _name{ std::move(name) }
                {
                }

                virtual type * get_type() const override
                {
                    if (_type)
                    {
                        return _type->get_value();
                    }

                    assert(0);
                }

                void set_type(variable * type)
                {
                    assert(type && type->get_type() == builtin_types().type.get());
                    _type = dynamic_cast<type_variable *>(type);
                }

            private:
                // TODO: there's a chance this should be an assert
                // and that this all should be replaced during simplification
                // but for now, this also has to work
                virtual variable_ir _codegen_ir(ir_generation_context & ctx) const override
                {
                    assert(_type && _type->get_value());
                    return {
                        codegen::ir::make_variable(
                            _type->get_value()->codegen_type(ctx),
                            _name
                        )
                    };
                }

                type_variable * _type = nullptr;
                std::u32string _name;
            };

            inline std::unique_ptr<unresolved_variable> make_unresolved_variable(std::u32string name)
            {
                return std::make_unique<unresolved_variable>(std::move(name));
            }
        }}
    }
}

