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
#include <string>
#include <vector>

#include <reaver/variant.h>

#include "function.h"
#include "scope.h"

namespace reaver::vapor::codegen
{
inline namespace _v1
{
    namespace ir
    {
        struct variable_type;

        struct member_variable
        {
            std::u32string name;
            std::shared_ptr<variable_type> type;
            std::size_t offset;
        };

        using member = std::variant<member_variable, codegen::ir::function>;

        struct variable_type
        {
            variable_type(std::u32string name = {},
                std::vector<scope> scopes = {},
                std::size_t size = {},
                std::vector<member> members = {})
                : name{ std::move(name) },
                  scopes{ std::move(scopes) },
                  size{ size },
                  members{ std::move(members) }
            {
            }

            virtual ~variable_type() = default;

            std::u32string name;
            std::vector<scope> scopes;
            std::size_t size;
            std::vector<member> members;
        };

        struct sized_integer_type : variable_type
        {
            using variable_type::variable_type;

            std::size_t integer_size;
        };

        inline const auto & builtin_types()
        {
            struct builtin_types_t
            {
                std::shared_ptr<variable_type> integer;
                std::shared_ptr<variable_type> boolean;
                std::shared_ptr<variable_type> type;

                std::shared_ptr<variable_type> sized_integer(std::size_t size) const
                {
                    static std::unordered_map<std::size_t, std::shared_ptr<variable_type>> types;

                    auto & type = types[size];
                    if (!type)
                    {
                        auto sized_type = std::make_shared<sized_integer_type>(
                            U"sized_integer_" + utf32(std::to_string(size)));
                        sized_type->integer_size = size;
                        type = sized_type;
                    }
                    return type;
                }
            };

            static auto types = [] {
                builtin_types_t types;
                types.integer = std::make_shared<variable_type>();
                types.integer->name = U"int";
                types.boolean = std::make_shared<variable_type>();
                types.boolean->name = U"bool";
                types.type = std::make_shared<variable_type>();
                types.type->name = U"type";
                return types;
            }();

            return types;
        }
    }
}
}
