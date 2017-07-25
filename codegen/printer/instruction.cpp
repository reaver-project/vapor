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

#include "vapor/codegen/ir/instruction.h"
#include "vapor/codegen/printer.h"

namespace reaver::vapor::codegen
{
inline namespace _v1
{
    std::u32string ir_printer::generate(const ir::instruction & inst, codegen_context & ctx)
    {
        std::u32string ret;

        if (inst.label)
        {
            ret += U"label `" + inst.label.get() + U"`:\n";
        }

        if (inst.declared_variable)
        {
            ret += generate_definition(*inst.declared_variable.get(), ctx);
        }

        ret += _to_string(inst.result) + U" = " + boost::locale::conv::utf_to_utf<char32_t>(inst.instruction.explain()) + U" ";
        ret += boost::algorithm::join(fmap(inst.operands, [&](auto && v) { return _to_string(v); }), U", ");
        ret += U"\n";

        return ret;
    }
}
}
