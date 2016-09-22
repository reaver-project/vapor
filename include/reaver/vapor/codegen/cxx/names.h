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

namespace reaver
{
    namespace vapor
    {
        namespace codegen { inline namespace _v1
        {
            namespace ir
            {
                struct variable_type;
                struct function;
                struct variable;
            }

            class codegen_context;

            namespace cxx
            {
                std::u32string type_name(const std::shared_ptr<ir::variable_type> &, codegen_context &);
                std::u32string function_name(const ir::function &, codegen_context &);
                std::u32string variable_name(const ir::variable &, codegen_context &);
            }
        }}
    }
}

