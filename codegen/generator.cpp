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

#include "vapor/codegen/generator.h"
#include "vapor/codegen/ir/type.h"

#include <cassert>

namespace reaver::vapor::codegen
{
inline namespace _v1
{
    std::u32string codegen_context::declare_if_necessary(std::shared_ptr<ir::variable_type> type)
    {
        if (_declared_types.find(type) != _declared_types.end())
        {
            return {};
        }

        _declared_types.insert(type);
        return _generator->generate_declaration(type, *this);
    }

    std::u32string codegen_context::define_if_necessary(std::shared_ptr<ir::variable_type> type)
    {
        if (_defined_types.find(type) != _defined_types.end())
        {
            return {};
        }

        _defined_types.insert(type);
        return _generator->generate_definition(type, *this);
    }
}
}
