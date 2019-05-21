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

#include "vapor/codegen/ir/type.h"
#include "vapor/codegen/llvm_ir.h"

namespace reaver::vapor::codegen
{
inline namespace _v1
{
    std::u32string llvm_ir_generator::generate_definition(ir::variable & var, codegen_context & ctx)
    {
        if (var.type == ir::builtin_types().type)
        {
            assert(var.initializer);
            auto refers_to = std::get_if<std::shared_ptr<ir::type>>(var.initializer.value().operator->());
            assert(refers_to);
            ctx.put_into_global_before += ctx.define_if_necessary(*refers_to);
            return {};
        }

        std::u32string ret;

        ret += ctx.define_if_necessary(var.type);

        assert(!ctx.in_function_definition);

        std::u32string initializer = var.imported
            ? U""
            : fmap(var.initializer, [&](auto && init) { return value_of(init, ctx); }).value_or(U"{ }");
        ret += variable_name(var, ctx) + U" = " + (var.imported ? U"external " : U"")
            + (var.constant ? U"constant " : U"global ") + type_name(var.type, ctx) + U" " + initializer
            + U"\n\n";

        return ret;
    }
}
}