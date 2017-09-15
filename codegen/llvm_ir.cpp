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

#include "vapor/codegen/llvm_ir.h"
#include "vapor/codegen/ir/module.h"
#include "vapor/codegen/ir/type.h"

namespace reaver::vapor::codegen
{
inline namespace _v1
{
    std::u32string llvm_ir_generator::generate_global_definitions(codegen_context &) const
    {
        return UR"code(target triple = "x86_64-pc-linux-gnu"

)code";
    }

    std::u32string llvm_ir_generator::generate_definitions(ir::module & module, codegen_context & ctx)
    {
        std::u32string ret;

        for (auto && symbol : module.symbols)
        {
            ret += get<0>(fmap(symbol,
                make_overload_set([&](std::shared_ptr<ir::variable> & var) { return this->generate_definition(*var, ctx); },
                    [&](ir::function & fn) { return this->generate_definition(fn, ctx); })));
        }

        return ret;
    }

    std::u32string llvm_ir_generator::type_name(std::shared_ptr<ir::variable_type> type, codegen_context & ctx)
    {
        if (type == ir::builtin_types().integer)
        {
            assert(0);
        }

        if (type == ir::builtin_types().boolean)
        {
            return U"i1";
        }

        if (auto sized = dynamic_cast<const ir::sized_integer_type *>(type.get()))
        {
            return U"i" + utf32(std::to_string(sized->integer_size));
        }

        ctx.put_into_global_before += ctx.define_if_necessary(type);
        return U"%\"" + type->name + U"\"";
    }

    std::u32string llvm_ir_generator::function_name(ir::function & fn, codegen_context & ctx)
    {
        return fn.name;
    }

    std::u32string llvm_ir_generator::variable_name(ir::variable & var, codegen_context & ctx)
    {
        if (!var.name)
        {
            var.name = utf32(std::to_string(ctx.unnamed_variable_index++));
        }

        return (ctx.in_function_definition ? U"%\"" : U"@\"") + var.name.get() + U"\"";
    }
}
}
