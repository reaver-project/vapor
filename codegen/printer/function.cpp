/**
 * Vapor Compiler Licence
 *
 * Copyright © 2017 Michał "Griwes" Dominiak
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

#include "vapor/codegen/ir/function.h"
#include "vapor/codegen/printer.h"

namespace reaver::vapor::codegen
{
inline namespace _v1
{
    std::u32string ir_printer::generate_definition(const ir::function & fn, codegen_context & ctx)
    {
        std::u32string ret;

        if (auto type = fn.parent_type.lock())
        {
            if (type != ctx.declaring_members_for)
            {
                return {};
            }
        }

        fmap(fn.parameters, [&](auto && value) {
            ctx.put_into_global_before += ctx.define_if_necessary(ir::get_type(value));
            return unit{};
        });

        ret += U"define function @ " + _pointer_to_string(&fn) + U" `" + fn.name + U"`:\n{\n";

        ret += U"}\n";

        return ret;
    }
}
}
