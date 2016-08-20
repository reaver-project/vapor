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

#include "../lexer/token.h"
#include "scope.h"

namespace reaver
{
    namespace vapor
    {
        namespace analyzer { inline namespace _v1
        {
            class function;

            class type : public std::enable_shared_from_this<type>
            {
            public:
                virtual ~type() = default;

                virtual std::shared_ptr<function> get_overload(lexer::token_type, std::shared_ptr<type>) const
                {
                    return nullptr;
                }

                virtual std::string explain() const
                {
                    return "type";
                }
            };

            inline std::shared_ptr<function> resolve_overload(const std::shared_ptr<type> & lhs, const std::shared_ptr<type> & rhs, lexer::token_type op, std::shared_ptr<scope> in_scope)
            {
                auto overload = lhs->get_overload(op, rhs);
                if (overload)
                {
                    return overload;
                }

                /*overload = in_scope->get_ref(op)->get_overload(lhs, rhs);
                if (overload)
                {
                    return overload;
                }*/

                logger::dlog() << lhs << " " << lexer::token_types[+op] << " " << rhs << " = ?";
                assert(0);
            }

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

