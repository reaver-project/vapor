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

#include <reaver/variant.h>

namespace reaver
{
    namespace vapor
    {
        namespace analyzer { inline namespace _v1
        {
            class type : public std::enable_shared_from_this<type>
            {
            public:
                virtual bool is_resolved() const
                {
                    return true;
                }

                virtual std::shared_ptr<type> try_resolve()
                {
                    return shared_from_this();
                }
            };

            class expression;

            class unresolved_type : public type
            {
            public:
                unresolved_type(std::shared_ptr<expression> expr) : _state{ expr }
                {
                }

                virtual bool is_resolved() const override
                {
                    return _state.index() == 0;
                }

                virtual std::shared_ptr<type> try_resolve() override
                {
                    return get<0>(fmap(_state, make_overload_set(
                        [&](std::shared_ptr<type> ready) {
                            return ready;
                        },

                        [&](std::shared_ptr<expression> expr) {
                            assert(0);
                            return std::shared_ptr<type>();
                        }
                    )));
                }

            private:
                variant<
                    std::shared_ptr<type>,
                    std::shared_ptr<expression>
                > _state;
            };

            std::shared_ptr<type> make_integer_type();

            inline const auto & builtin_types()
            {
                struct builtin_types_t
                {
                    using member_t = std::shared_ptr<type>;

                    member_t type;
                    member_t integer;
                };

                static builtin_types_t builtins;

                builtins.type = std::make_shared<type>();
                builtins.integer = make_integer_type();

                return builtins;
            }
        }}
    }
}

