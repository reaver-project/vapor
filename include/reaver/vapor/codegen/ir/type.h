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
#include <string>
#include <vector>

#include <reaver/variant.h>

#include "scope.h"
#include "function.h"

namespace reaver
{
    namespace vapor
    {
        namespace codegen { inline namespace _v1
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

                using member = variant<member_variable, codegen::ir::function>;

                struct variable_type
                {
                    std::u32string name;
                    std::vector<scope> scopes;
                    std::size_t size;
                    std::vector<member> members;
                };

                inline std::ostream & operator<<(std::ostream & os, const variable_type & type)
                {
                    if (type.members.empty())
                    {
                        return os;
                    }

                    os << "type\n{\n";
                    os << "name: " << utf8(type.name) << "\n";
                    os << "<print scopes one day!>\n";
                    os << "size: " << type.size << "\n";

                    fmap(type.members, [&](auto && member) {
                        fmap(member, make_overload_set(
                            [&](const function & fun) {
                                os << fun << '\n';
                                return unit{};
                            },
                            [&](auto &&) {
                                assert(0);
                                return unit{};
                            }
                        ));
                        return unit{};
                    });

                    os << "}\n";
                    return os;
                }

                inline auto make_type(std::u32string name, std::vector<scope> scopes, std::size_t size, std::vector<member> members)
                {
                    return std::make_shared<variable_type>(variable_type{ std::move(name), std::move(scopes), size, std::move(members) });
                }

                inline const auto & builtin_types()
                {
                    struct builtin_types_t
                    {
                        std::shared_ptr<variable_type> integer;
                        std::shared_ptr<variable_type> boolean;
                    };

                    static auto types = []{
                        builtin_types_t types;
                        types.integer = std::make_shared<variable_type>();
                        types.integer->name = U"int";
                        types.boolean = std::make_shared<variable_type>();
                        types.boolean->name = U"bool";
                        return types;
                    }();

                    return types;
                }
            }
        }}
    }
}

