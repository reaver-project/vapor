/**
 * Vapor Compiler Licence
 *
 * Copyright © 2014-2016 Michał "Griwes" Dominiak
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

#include "../range.h"
#include "statement.h"
#include "helpers.h"
#include "id_expression.h"

namespace reaver
{
    namespace vapor
    {
        namespace parser { inline namespace _v1
        {
            struct module
            {
                range_type range;
                id_expression name;
                std::vector<statement> statements;
            };

            module parse_module(context & ctx)
            {
                module ret;

                auto start = expect(ctx, lexer::token_type::module).range.start();
                ret.name = parse_id_expression(ctx);

                expect(ctx, lexer::token_type::curly_bracket_open);

                while (!peek(ctx, lexer::token_type::curly_bracket_close))
                {
                    ret.statements.push_back(parse_statement(ctx));
                }

                auto end = expect(ctx, lexer::token_type::curly_bracket_close).range.end();

                ret.range = { start, end };

                return ret;
            }

            void print(const module & mod, std::ostream & os, std::size_t indent = 0)
            {
                auto in = std::string(indent, ' ');

                os << in << "`module` at " << mod.range << '\n';
                os << in << "{\n";
                print(mod.name, os, indent + 4);
                os << in << "}\n";

                os << in << "{\n";
                {
                    auto in = std::string(indent + 4, ' ');
                    for (auto && statement : mod.statements)
                    {
                        os << in << "{\n";
                        print(statement, os, indent + 8);
                        os << in << "}\n";
                    }
                }
                os << in << "}\n";
            }
        }}
    }
}

