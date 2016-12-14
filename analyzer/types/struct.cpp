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

#include "vapor/analyzer/types/struct.h"
#include "vapor/analyzer/statements/declaration.h"
#include "vapor/analyzer/symbol.h"
#include "vapor/parser/expr.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    struct_type::struct_type(const parser::struct_literal & parse, scope * lex_scope) : type{ lex_scope }, _parse{ parse }
    {
        _members = fmap(parse.members, [&](auto && member) {
            return get<0>(fmap(member,
                make_overload_set(
                    [&](const parser::declaration & decl) -> std::unique_ptr<statement> {
                        auto scope = _member_scope.get();
                        auto decl_stmt = preanalyze_member_declaration(decl, scope);
                        assert(scope == _member_scope.get());
                        return decl_stmt;
                    },

                    [&](const parser::function & func) -> std::unique_ptr<statement> { assert(0); })));
        });
    }
}
}
