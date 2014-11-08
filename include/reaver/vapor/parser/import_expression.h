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

#include "vapor/range.h"
#include "vapor/parser/helpers.h"
#include "vapor/parser/id_expression.h"

namespace reaver
{
    namespace vapor
    {
        namespace parser { inline namespace _v1
        {
            struct import_expression
            {
                class range range;
                id_expression module_name;
            };

            template<typename Context>
            auto parse_import_expression(Context & ctx)
            {
                import_expression ret;

                auto start = expect(ctx, lexer::token_type::import).range.start();
                ret.module_name = parse_id_expression(ctx);
                ret.range = { start, ret.module_name.range.end() };

                return ret;
            }

            void print(const import_expression & expr, std::ostream & os, std::size_t indent = 0)
            {
                auto in = std::string(indent, ' ');

                os << in << "`import-expression` at " << expr.range << '\n';
                os << in << "{\n";
                print(expr.module_name, os, indent + 4);
                os << in << "}\n";
            }
        }}
    }
}
