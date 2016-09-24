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
#include "vapor/codegen/ir/variable.h"
#include "vapor/codegen/ir/type.h"
#include "vapor/codegen/cxx/names.h"

#include <cassert>

std::u32string reaver::vapor::codegen::_v1::cxx_generator::generate_declaration(reaver::vapor::codegen::_v1::ir::variable & var, reaver::vapor::codegen::_v1::codegen_context & ctx) const
{
    std::u32string ret;

    ret += ctx.declare_if_necessary(var.type);
    ret += U"extern " + cxx::type_name(var.type, ctx) + U" " + cxx::declaration_variable_name(var, ctx) + U";\n";

    return ret;
}

std::u32string reaver::vapor::codegen::_v1::cxx_generator::generate_definition(const reaver::vapor::codegen::_v1::ir::variable & var, reaver::vapor::codegen::_v1::codegen_context & ctx) const
{
    std::u32string ret;

    ret += ctx.define_if_necessary(var.type);
    ret += cxx::type_name(var.type, ctx) + U" " + cxx::variable_name(var, ctx) + U"{};\n";

    return ret;
}

std::u32string reaver::vapor::codegen::_v1::cxx::value_of(const reaver::vapor::codegen::_v1::ir::value & val, reaver::vapor::codegen::_v1::codegen_context & ctx)
{
    return get<std::u32string>(fmap(val, make_overload_set(
        [&](const codegen::ir::integer_value & val) {
            std::ostringstream os;
            os << val;
            return boost::locale::conv::utf_to_utf<char32_t>(os.str());
        },
        [&](const std::shared_ptr<ir::variable> & var) {
            return variable_name(*var, ctx);
        },
        [&](auto &&) {
            assert(0);
            return unit{};
        }
    )));
}
