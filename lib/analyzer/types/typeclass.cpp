/**
 * Vapor Compiler Licence
 *
 * Copyright © 2017-2019 Michał "Griwes" Dominiak
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

#include "vapor/analyzer/types/typeclass.h"

#include <boost/algorithm/string/join.hpp>

#include "expressions/type.pb.h"
#include "type_reference.pb.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    std::string typeclass_type::explain() const
    {
        return "typeclass ("
            + boost::join(fmap(_param_types, [](auto && param) { return param->explain(); }), ",") + ")";
    }

    void typeclass_type::print(std::ostream & os, print_context ctx) const
    {
        os << styles::def << ctx << styles::type << explain() << styles::def << " @ " << styles::address
           << this << styles::def << ": builtin type\n";
    }

    std::unique_ptr<proto::type> typeclass_type::generate_interface() const
    {
        auto ret = std::make_unique<proto::type>();
        ret->set_allocated_reference(generate_interface_reference().release());
        return ret;
    }

    std::unique_ptr<proto::type_reference> typeclass_type::generate_interface_reference() const
    {
        auto ret = std::make_unique<proto::type_reference>();

        auto builtin_tc_ref = std::make_unique<proto::typeclass_type>();
        for (auto && param : _param_types)
        {
            builtin_tc_ref->add_parameters()->set_allocated_type(
                param->generate_interface_reference().release());
        }

        ret->set_allocated_typeclass(builtin_tc_ref.release());
        return ret;
    }
}
}
