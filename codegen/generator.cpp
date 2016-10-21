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

#include "vapor/codegen/generator.h"
#include "vapor/codegen/ir/type.h"
#include "vapor/codegen/cxx/names.h" // baaaaad, need to sort this nonsense out

#include <cassert>

std::u32string reaver::vapor::codegen::_v1::codegen_context::declare_if_necessary(std::shared_ptr<reaver::vapor::codegen::_v1::ir::variable_type> type)
{
    if (_declared_types.find(type) != _declared_types.end())
    {
        return {};
    }

    _declared_types.insert(type);
    return _generator->generate_declaration(type, *this);
}

std::u32string reaver::vapor::codegen::_v1::codegen_context::define_if_necessary(std::shared_ptr<reaver::vapor::codegen::_v1::ir::variable_type> type)
{
    if (_defined_types.find(type) != _defined_types.end())
    {
        return {};
    }

    _defined_types.insert(type);
    return _generator->generate_definition(type, *this);
}

std::u32string reaver::vapor::codegen::_v1::codegen_context::get_storage_for(std::shared_ptr<reaver::vapor::codegen::_v1::ir::variable_type> type)
{
    auto it = _unallocated_variables.find(type);
    if (it != _unallocated_variables.end())
    {
        if (!it->second.empty())
        {
            auto ret = std::move(it->second.back());
            it->second.pop_back();
            return ret;
        }
    }

    auto var = U"__pseudoregister_" + boost::locale::conv::utf_to_utf<char32_t>(std::to_string(storage_object_index++));
    put_into_function_header += U"::reaver::manual_object<" + cxx::type_name(type, *this) + U"> " + var + U";\n";
    return var;
}

void reaver::vapor::codegen::_v1::codegen_context::free_storage_for(std::u32string name, std::shared_ptr<reaver::vapor::codegen::_v1::ir::variable_type> type)
{
    _unallocated_variables[std::move(type)].push_back(std::move(name));
}

