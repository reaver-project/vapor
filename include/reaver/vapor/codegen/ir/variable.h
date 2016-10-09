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

#include <reaver/variant.h>
#include <reaver/optional.h>

#include "integer.h"
#include "boolean.h"
#include "../../utf8.h"
#include "scope.h"

namespace reaver
{
    namespace vapor
    {
        namespace codegen { inline namespace _v1
        {
            namespace ir
            {
                struct variable_type;

                struct variable
                {
                    std::shared_ptr<variable_type> type;
                    optional<std::u32string> name;
                    bool declared = false;
                    bool argument = false;
                    std::vector<scope> scopes = {};
                };

                struct label
                {
                    std::u32string name;
                    std::vector<scope> scopes;
                };

                inline std::shared_ptr<variable> make_variable(std::shared_ptr<variable_type> type, optional<std::u32string> name = none)
                {
                    return std::make_shared<variable>(variable{ std::move(type), std::move(name) });
                }

                std::ostream & operator<<(std::ostream & os, const variable & var);

                using value = variant<
                    std::shared_ptr<variable>,
                    integer_value,
                    boolean_value,
                    label
                >;

                std::ostream & operator<<(std::ostream & os, const value & val);
                std::shared_ptr<variable_type> get_type(const value &);
            }
        }}
    }
}

