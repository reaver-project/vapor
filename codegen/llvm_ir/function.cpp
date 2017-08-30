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
#include "vapor/codegen/llvm_ir.h"

namespace reaver::vapor::codegen
{
inline namespace _v1
{
    std::u32string llvm_ir_generator::generate_definition(ir::function & fn, codegen_context & ctx)
    {
        std::u32string ret;

        if (auto type = fn.parent_type.lock())
        {
            ret += ctx.define_if_necessary(type);
        }

        ret += ctx.define_if_necessary(ir::get_type(fn.return_value));
        for (auto && param : fn.parameters)
        {
            ret += ctx.define_if_necessary(ir::get_type(param));
        }

        ret += U"define " + type_name(ir::get_type(fn.return_value), ctx);
        ret += U" @" + function_name(fn, ctx);
        ret += U"(\n";
        for (auto && param : fn.parameters)
        {
            ret += U"    " + type_name(ir::get_type(param), ctx) + U" " + variable_name(*param, ctx, true) + U",\n";
        }

        if (!fn.parameters.empty())
        {
            ret.pop_back();
            ret.pop_back();
            ret.push_back(U'\n');
        }
        ret += U")\n{\n";

        // there needs to be an entry label
        ret += U"entry:\n";

        for (auto && inst : fn.instructions)
        {
            ret += generate(inst, ctx);
        }

        ret += U"}\n";

        return ret;
    }
}
}
