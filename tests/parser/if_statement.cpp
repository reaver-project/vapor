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

#include <reaver/mayfly.h>

#include "helpers.h"

using namespace reaver::vapor;
using namespace reaver::vapor::parser;

MAYFLY_BEGIN_SUITE("parser");
MAYFLY_BEGIN_SUITE("if_statement");

MAYFLY_ADD_TESTCASE("if-then",
    test(UR"(if (true) { return 1; })",
        if_statement{ { 0, 23 },
            { { 4, 8 },
                boolean_literal{ { 4, 8 }, { lexer::token_type::boolean, UR"(true)", { 4, 8 } }, {} } },
            block{ { 10, 23 },
                { statement{ { 12, 21 },
                    return_expression{ { 12, 20 },
                        { { 19, 20 },
                            integer_literal{ { 19, 20 },
                                { lexer::token_type::integer, UR"(1)", { 19, 20 } },
                                {} } } } } },
                {} },
            {} },
        &parse_if_statement));

MAYFLY_ADD_TESTCASE("if-then-else",
    test(UR"(if (true) { return 1; } else { return 2; })",
        if_statement{ { 0, 42 },
            { { 4, 8 },
                boolean_literal{ { 4, 8 }, { lexer::token_type::boolean, UR"(true)", { 4, 8 } }, {} } },
            block{ { 10, 23 },
                { statement{ { 12, 21 },
                    return_expression{ { 12, 20 },
                        { { 19, 20 },
                            integer_literal{ { 19, 20 },
                                { lexer::token_type::integer, UR"(1)", { 19, 20 } },
                                {} } } } } },
                {} },
            std::make_optional(reaver::recursive_wrapper<block>{ block{ { 29, 42 },
                { statement{ { 31, 40 },
                    return_expression{ { 31, 39 },
                        { { 38, 39 },
                            integer_literal{ { 38, 39 },
                                { lexer::token_type::integer, UR"(2)", { 38, 39 } },
                                {} } } } } },
                {} } }) },
        &parse_if_statement));

MAYFLY_END_SUITE;
MAYFLY_END_SUITE;
