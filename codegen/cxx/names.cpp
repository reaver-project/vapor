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

#include "vapor/codegen/cxx/names.h"
#include "vapor/codegen/generator.h"
#include "vapor/codegen/ir/function.h"
#include "vapor/codegen/ir/type.h"
#include "vapor/codegen/ir/variable.h"

#include <cassert>

namespace reaver::vapor::codegen
{
inline namespace _v1
{
    std::u32string cxx::type_name(const std::shared_ptr<const ir::variable_type> & type, codegen_context &)
    {
        if (type == ir::builtin_types().integer)
        {
            return U"::boost::multiprecision::cpp_int";
        }

        if (auto sized = dynamic_cast<const ir::sized_integer_type *>(type.get()))
        {
            if (sized->integer_size <= 8)
            {
                return U"std::int_least8_t";
            }

            if (sized->integer_size <= 16)
            {
                return U"std::int_least16_t";
            }

            if (sized->integer_size <= 32)
            {
                return U"std::int_least32_t";
            }

            if (sized->integer_size <= 64)
            {
                return U"std::int_least64_t";
            }

            return {};
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

    std::u32string cxx::declaration_type_name(const std::shared_ptr<ir::variable_type> & type, codegen_context &)
    {
        return type->name;
    }

    std::u32string cxx::variable_name(const ir::variable & var, codegen_context & ctx)
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

        logger::default_logger().sync();
        assert(!"non-declaration name for unnamed variable requested");
        // return U"__unnamed_variable_" + utf32(std::to_string(ctx.unnamed_variable_index++));
    }

    std::u32string cxx::declaration_variable_name(ir::variable & var, codegen_context & ctx)
    {
        if (var.name)
        {
            return *var.name;
        }

        auto name = U"__unnamed_variable_" + utf32(std::to_string(ctx.unnamed_variable_index++));
        var.name = name;
        return name;
    }

    std::u32string cxx::function_name(const ir::function & fn, codegen_context &)
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

    std::u32string cxx::declaration_function_name(const ir::function & fn, codegen_context &)
    {
        return fn.name;
    }
}
}
