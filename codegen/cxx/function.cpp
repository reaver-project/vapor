/**
 * Vapor Compiler Licence
 *
 * Copyright © 2016-2017 Michał "Griwes" Dominiak
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
#include "vapor/codegen/cxx.h"
#include "vapor/codegen/cxx/names.h"
#include "vapor/codegen/ir/variable.h"

namespace reaver::vapor::codegen
{
inline namespace _v1
{
    std::u32string cxx_generator::generate_declaration(ir::function & fn, codegen_context & ctx) const
    {
        std::u32string ret;

        if (auto type = fn.parent_type.lock())
        {
            ret += ctx.define_if_necessary(type);

            if (ctx.declaring_members_for != type)
            {
                return ret;
            }
        }
        ret += ctx.declare_if_necessary(ir::get_type(fn.return_value));
        fmap(fn.parameters, [&](auto && value) {
            ret += ctx.declare_if_necessary(ir::get_type(value));
            return unit{};
        });

        ret += cxx::type_name(ir::get_type(fn.return_value), ctx);
        ret += U" ";
        ret += cxx::declaration_function_name(fn, ctx);
        ret += U"(\n";
        fmap(fn.parameters, [&](auto && var) {
            ret += U"    " + cxx::type_name(ir::get_type(var), ctx);
            ret += U" ";
            var->declared = true;
            cxx::declaration_variable_name(*var, ctx); // HAAAAACK
            ret += cxx::variable_name(*var, ctx);
            ret += U",\n";
            return unit{};
        });
        if (!fn.parameters.empty())
        {
            ret.pop_back();
            ret.pop_back();
            ret.push_back(U'\n');
        }
        ret += U");\n";

        if (!fn.parent_type.lock())
        {
            ctx.put_into_global += std::move(ret);
            ret = U"";
        }

        return ret;
    }

    std::u32string cxx_generator::generate_definition(const ir::function & fn, codegen_context & ctx)
    {
        std::u32string header;

        if (auto type = fn.parent_type.lock())
        {
            if (type != ctx.declaring_members_for)
            {
                return {};
            }
        }
        header += ctx.declare_if_necessary(ir::get_type(fn.return_value));
        fmap(fn.parameters, [&](auto && value) {
            header += ctx.declare_if_necessary(ir::get_type(value));
            return unit{};
        });

        header += cxx::type_name(ir::get_type(fn.return_value), ctx);
        header += U" ";
        header += cxx::function_name(fn, ctx);
        header += U"(\n";
        fmap(fn.parameters, [&](auto && var) {
            header += U"    " + cxx::type_name(ir::get_type(var), ctx);
            header += U" ";
            header += cxx::variable_name(*var, ctx);
            header += U",\n";

            var->argument = true;
            return unit{};
        });
        if (!fn.parameters.empty())
        {
            header.pop_back();
            header.pop_back();
            header.push_back(U'\n');
        }
        header += U")\n{\n";

        // fix phi variable names
        fmap(fn.instructions, [&](auto && inst) {
            if (inst.instruction.template is<ir::phi_instruction>())
            {
                assert(inst.label);
                std::u32string var_name = U"__phi_variable" + *inst.label;

                for (std::size_t i = 0; i < inst.operands.size() / 2; ++i)
                {
                    auto && result = inst.operands[i * 2 + 1];
                    assert(result.index() == 0);

                    auto && var = *get<std::shared_ptr<ir::variable>>(result);
                    var.name = var_name;
                    var.declared = true;
                }
            }

            return unit{};
        });

        std::u32string body;

        fmap(fn.instructions, [&](auto && inst) {
            body += this->generate(inst, ctx);
            return unit{};
        });
        body += U"};\n";

        auto ret = header + ctx.put_into_function_header + body;
        ctx.put_into_function_header.clear();
        auto cxxgen = dynamic_cast<cxx_generator &>(ctx.generator());
        cxxgen.clear_storage();
        return ret;
    }
}
}
