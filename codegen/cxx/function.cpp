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

#include "vapor/codegen/cxx.h"
#include "vapor/codegen/ir/function.h"
#include "vapor/codegen/cxx/names.h"

std::u32string reaver::vapor::codegen::_v1::cxx_generator::generate_declaration(const reaver::vapor::codegen::_v1::ir::function & fn, reaver::vapor::codegen::_v1::codegen_context & ctx) const
{
    std::u32string ret;

    if (auto type = fn.parent_type.lock())
    {
        ret += ctx.declare_if_necessary(type);
    }
    ret += ctx.declare_if_necessary(ir::get_type(fn.return_value));
    fmap(fn.arguments, [&](auto && value) {
        ret += ctx.declare_if_necessary(ir::get_type(value));
        return unit{};
    });

    ret += cxx::type_name(ir::get_type(fn.return_value), ctx);
    ret += U" ";
    ret += cxx::declaration_function_name(fn, ctx);
    ret += U"(\n";
    fmap(fn.arguments, [&](auto && var) {
        ret += U"    " + cxx::type_name(ir::get_type(var), ctx);
        ret += U" ";
        ret += cxx::variable_name(*var, ctx);
        ret += U",\n";
        return unit{};
    });
    if (!fn.arguments.empty())
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

std::u32string reaver::vapor::codegen::_v1::cxx_generator::generate_definition(const reaver::vapor::codegen::_v1::ir::function & fn, reaver::vapor::codegen::_v1::codegen_context & ctx) const
{
    std::u32string ret;

    if (auto type = fn.parent_type.lock())
    {
        ret += ctx.declare_if_necessary(type);
    }
    ret += ctx.declare_if_necessary(ir::get_type(fn.return_value));
    fmap(fn.arguments, [&](auto && value) {
        ret += ctx.declare_if_necessary(ir::get_type(value));
        return unit{};
    });

    ret += cxx::type_name(ir::get_type(fn.return_value), ctx);
    ret += U" ";
    ret += cxx::function_name(fn, ctx);
    ret += U"(\n";
    fmap(fn.arguments, [&](auto && var) {
        ret += U"    " + cxx::type_name(ir::get_type(var), ctx);
        ret += U" ";
        ret += cxx::variable_name(*var, ctx);
        ret += U",\n";
        return unit{};
    });
    if (!fn.arguments.empty())
    {
        ret.pop_back();
        ret.pop_back();
        ret.push_back(U'\n');
    }
    ret += U")\n{\n";
    fmap(fn.instructions, [&](auto && inst) {
        ret += generate(inst, ctx);
        return unit{};
    });
    ret += U"};\n";

    return ret;
}

