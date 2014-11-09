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

#include <string>

#include "vapor/range.h"
#include "vapor/parser/expression_list.h"

namespace reaver
{
    namespace vapor
    {
        namespace parser { inline namespace _v1
        {
            struct declaration
            {
                class range range;
                lexer::token identifier;
                expression_list rhs;
            };

            template<typename Context>
            declaration parse_declaration(Context & ctx)
            {
                declaration ret;

                auto start = expect(ctx, lexer::token_type::auto_).range.start();
                ret.identifier = std::move(expect(ctx, lexer::token_type::identifier));
                expect(ctx, lexer::token_type::assign);
                ret.rhs = parse_expression_list(ctx);
                ret.range = { start, ret.rhs.range.end() };

                return ret;
            }

            void print(const declaration & decl, std::ostream & os, std::size_t indent = 0)
            {
                auto in = std::string(indent, ' ');

                os << in << "`declaration` at " << decl.range << '\n';

                os << in << "{\n";
                print(decl.identifier, os, indent + 4);
                print(decl.rhs, os, indent + 4);
                os << in << "}\n";
            }
        }}
    }
}
