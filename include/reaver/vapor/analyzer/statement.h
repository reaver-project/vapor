/**
 * Vapor Compiler Licence
 *
 * Copyright © 2014 Michał "Griwes" Dominiak
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

#pragma once

#include <boost/variant.hpp>

#include "vapor/parser/statement.h"
#include "vapor/analyzer/declaration.h"
#include "vapor/analyzer/import.h"
#include "vapor/analyzer/expression.h"
#include "vapor/analyzer/helpers.h"

namespace reaver
{
    namespace vapor
    {
        namespace analyzer { inline namespace _v1
        {
            using statement = shptr_variant<declaration, import, expression>;

            statement preanalyze_statement(const parser::statement & parse, const std::shared_ptr<scope> & lex_scope)
            {
                return visit(make_visitor(
                    id<parser::declaration>(), [&](auto && decl) -> statement
                    {
                        return make_declaration(decl.identifier.string, preanalyze_expression(decl.rhs, lex_scope), lex_scope, decl);
                    },

                    id<parser::return_expression>(), [](auto && ret_expr) -> statement
                    {
                        assert(0);
                        return std::shared_ptr<import>();
                    },

                    id<parser::expression_list>(), [](auto && expr_list) -> statement
                    {
                        assert(0);
                        return std::shared_ptr<expression>();
                    },

                    default_id(), [](auto &&) -> statement { assert(0); }
                ), parse.statement_value);
            }
        }}
    }
}
