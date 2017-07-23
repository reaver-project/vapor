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

#include <boost/algorithm/string/join.hpp>

#include "vapor/codegen/cxx.h"
#include "vapor/codegen/cxx/names.h"
#include "vapor/codegen/ir/type.h"
#include "vapor/codegen/ir/variable.h"

#include <cassert>

namespace reaver::vapor::codegen
{
inline namespace _v1
{
    std::u32string cxx_generator::generate_declaration(ir::variable & var, codegen_context & ctx) const
    {
        std::u32string ret;

        if (var.type == ir::builtin_types().type)
        {
            assert(var.refers_to);
            ctx.put_into_global_before += ctx.declare_if_necessary(var.refers_to);
            ret += U"using " + cxx::declaration_variable_name(var, ctx) + U" = " + var.refers_to->name + U";\n";
        }

        else
        {
            ret += ctx.declare_if_necessary(var.type);
            ret += U"extern " + cxx::type_name(var.type, ctx) + U" " + cxx::declaration_variable_name(var, ctx) + U";\n";
        }

        var.declared = true;

        return ret;
    }

    std::u32string cxx_generator::generate_definition(const ir::variable & var, codegen_context & ctx)
    {
        std::u32string ret;

        if (var.type == ir::builtin_types().type)
        {
            assert(var.refers_to);
            ctx.put_into_global_before += ctx.define_if_necessary(var.refers_to);
            return U"";
        }

        ctx.put_into_global_before += ctx.define_if_necessary(var.type);
        ret += cxx::type_name(var.type, ctx) + U" " + cxx::variable_name(var, ctx) + U"{};\n";

        return ret;
    }

    std::u32string cxx_generator::generate_definition(const ir::member_variable & member, codegen_context & ctx)
    {
        std::u32string ret;

        if (member.type == ir::builtin_types().type)
        {
            assert(0);
        }

        ctx.put_into_global_before += ctx.define_if_necessary(member.type);
        ret += cxx::type_name(member.type, ctx) + U" " + member.name + U"{};\n";

        return ret;
    }

    std::u32string cxx::value_of(const ir::value & val, codegen_context & ctx, bool dont_unref)
    {
        return get<std::u32string>(fmap(val,
            make_overload_set(
                [&](const codegen::ir::integer_value & val) {
                    std::ostringstream os;
                    os << val.value;
                    return boost::locale::conv::utf_to_utf<char32_t>(os.str());
                },
                [&](const codegen::ir::boolean_value & val) -> std::u32string { return val.value ? U"true" : U"false"; },
                [&](const std::shared_ptr<ir::variable> & var) {
                    return variable_name(*var, ctx) + (var->parameter || dont_unref ? U"" : var->is_move() ? U".move()" : U".reference()");
                },
                [&](const codegen::ir::label & label) {
                    assert(label.scopes.empty());
                    return label.name;
                },
                [&](const codegen::ir::struct_value & struct_val) {
                    std::vector<std::u32string> subvalues = fmap(struct_val.fields, [&](auto && val) { return value_of(val, ctx); });
                    return type_name(struct_val.type, ctx) + U"::constructor(" + boost::algorithm::join(subvalues, ", ") + U")";
                },
                [&](auto &&) {
                    assert(0);
                    return unit{};
                })));
    }

    std::u32string cxx::variable_of(const ir::value & val, codegen_context & ctx)
    {
        assert(val.index() == 0);
        return variable_name(*get<std::shared_ptr<ir::variable>>(val), ctx);
    }

    void cxx::mark_destroyed(const ir::value & val, codegen_context & ctx)
    {
        if (val.index() != 0)
        {
            return;
        }

        auto && var = *get<std::shared_ptr<ir::variable>>(val);
        if (!var.destroyed)
        {
            var.destroyed = true;
            auto cxxgen = dynamic_cast<cxx_generator &>(ctx.generator());
            cxxgen.free_storage_for(*var.name, var.type, ctx);
        }
    }
}
}
