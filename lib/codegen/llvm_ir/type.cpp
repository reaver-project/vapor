/**
 * Vapor Compiler Licence
 *
 * Copyright © 2017, 2019 Michał "Griwes" Dominiak
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
    std::u32string llvm_ir_generator::generate_definition(std::shared_ptr<ir::type> type,
        codegen_context & ctx)
    {
        if (type->is_fundamental())
        {
            return {};
        }

        if (auto user = dynamic_cast<ir::user_type *>(type.get()))
        {
            std::u32string ret;

            ret += type_name(type, ctx) + U" = type {";

            for (auto && member : user->members)
            {
                fmap(member,
                    make_overload_set(
                        [&](ir::member_variable & var) {
                            ret += U" " + type_name(var.type, ctx) + U",";
                            return unit{};
                        },
                        [&](ir::function & func) {
                            ctx.put_into_global += generate_definition(func, ctx);
                            return unit{};
                        }));
            }

            if (ret.back() == U',')
            {
                ret.pop_back();
            }

            ret += U" }\n\n";

            return ret;
        }

        assert(!"unsupported type in codegen ir!");
    }
}
}
