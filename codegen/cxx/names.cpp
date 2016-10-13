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

#include "vapor/codegen/cxx/names.h"
#include "vapor/codegen/ir/type.h"
#include "vapor/codegen/ir/variable.h"
#include "vapor/codegen/ir/function.h"
#include "vapor/codegen/generator.h"

#include <cassert>

std::u32string reaver::vapor::codegen::_v1::cxx::type_name(const std::shared_ptr<const reaver::vapor::codegen::_v1::ir::variable_type> & type, reaver::vapor::codegen::_v1::codegen_context &)
{
    if (type == ir::builtin_types().integer)
    {
        return U"::boost::multiprecision::cpp_int";
    }

    std::u32string ret;

    fmap(type->scopes, [&](auto && scope) {
        if (scope.type == ir::scope_type::function)
        {
            assert(0);
        }
        ret += scope.name + U"::";
        return unit{};
    });

    ret += type->name;
    return ret;
}

std::u32string reaver::vapor::codegen::_v1::cxx::declaration_type_name(const std::shared_ptr<reaver::vapor::codegen::_v1::ir::variable_type> & type, reaver::vapor::codegen::_v1::codegen_context &)
{
    return type->name;
}

std::u32string reaver::vapor::codegen::_v1::cxx::variable_name(const reaver::vapor::codegen::_v1::ir::variable & var, reaver::vapor::codegen::_v1::codegen_context & ctx)
{
    if (var.name)
    {
        std::u32string ret;

        fmap(var.scopes, [&](auto && scope) {
            if (scope.type == ir::scope_type::function)
            {
                assert(0);
            }
            ret += scope.name + U"::";
            return unit{};
        });

        return ret + *var.name;
    }

    return U"__unnamed_variable_" + boost::locale::conv::utf_to_utf<char32_t>(std::to_string(ctx.unnamed_variable_index++));
}

std::u32string reaver::vapor::codegen::_v1::cxx::declaration_variable_name(reaver::vapor::codegen::_v1::ir::variable & var, reaver::vapor::codegen::_v1::codegen_context & ctx)
{
    if (var.name)
    {
        return *var.name;
    }

    auto name = U"__unnamed_variable_" + boost::locale::conv::utf_to_utf<char32_t>(std::to_string(ctx.unnamed_variable_index++));
    var.name = name;
    return name;
}

std::u32string reaver::vapor::codegen::_v1::cxx::function_name(const reaver::vapor::codegen::_v1::ir::function & fn, reaver::vapor::codegen::_v1::codegen_context &)
{
    std::u32string ret;

    fmap(fn.scopes, [&](auto && scope) {
        if (scope.type == ir::scope_type::function)
        {
            assert(0);
        }
        ret += scope.name + U"::";
        return unit{};
    });

    ret += fn.name;
    return ret;
}

std::u32string reaver::vapor::codegen::_v1::cxx::declaration_function_name(const reaver::vapor::codegen::_v1::ir::function & fn, reaver::vapor::codegen::_v1::codegen_context &)
{
    return fn.name;
}

