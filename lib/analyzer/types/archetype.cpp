/**
 * Vapor Compiler Licence
 *
 * Copyright © 2019 Michał "Griwes" Dominiak
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

#include "vapor/analyzer/types/archetype.h"
#include "vapor/analyzer/semantic/symbol.h"

#include "type_reference.pb.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    archetype::archetype(ast_node node, const type * base, std::u32string param_name)
        : _node{ node }, _param_name{ std::move(param_name) }, _base_type{ base }
    {
    }

    std::string archetype::explain() const
    {
        assert(_typeclasses.empty());
        return std::string("archetype for `") + utf8(_param_name) + "` : " + _base_type->explain();
    }

    void archetype::print(std::ostream & os, print_context ctx) const
    {
        os << styles::def << ctx << styles::type << explain();
        print_address_range(os, this);
        os << '\n';
    }

    std::unique_ptr<proto::type_reference> archetype::generate_interface_reference() const
    {
        auto ret = std::make_unique<proto::type_reference>();

        auto arch = std::make_unique<proto::parameter_archetype>();
        arch->set_name(utf8(_param_name));

        ret->set_allocated_archetype(arch.release());
        return ret;
    }
}
}
