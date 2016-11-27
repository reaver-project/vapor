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

#include "vapor/codegen/ir/type.h"
#include "vapor/codegen/cxx.h"
#include "vapor/codegen/cxx/names.h"

#include <cassert>

namespace reaver::vapor::codegen
{
inline namespace _v1
{
    std::u32string cxx_generator::generate_declaration(const std::shared_ptr<ir::variable_type> & type, codegen_context & ctx) const
    {
        if (type == ir::builtin_types().integer)
        {
            ctx.put_into_global_before += UR"code(#include <boost/multiprecision/cpp_int.hpp>
    )code";
            return {};
        }

        if (type == ir::builtin_types().boolean)
        {
            return {};
        }

        return U"struct " + cxx::declaration_type_name(type, ctx) + U";\n";
    }

    std::u32string cxx_generator::generate_definition(const std::shared_ptr<ir::variable_type> & type, codegen_context & ctx) const
    {
        if (type == ir::builtin_types().integer)
        {
            return {};
        }

        if (type == ir::builtin_types().boolean)
        {
            return {};
        }

        std::u32string members;

        fmap(type->members, [&](auto && member) {
            fmap(member,
                make_overload_set(
                    [&](codegen::ir::function & fn) {
                        members += this->generate_declaration(fn, ctx);
                        ctx.put_into_global += this->generate_definition(fn, ctx);
                        return unit{};
                    },
                    [&](auto &&) {
                        assert(0);
                        return unit{};
                    }));
            return unit{};
        });

        return U"struct " + cxx::type_name(type, ctx) + U" {\n" + members + U"};\n";
    }
}
}
