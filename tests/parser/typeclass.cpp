/**
 * Vapor Compiler Licence
 *
 * Copyright © 2017 Michał "Griwes" Dominiak
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
MAYFLY_BEGIN_SUITE("typeclass");

MAYFLY_ADD_TESTCASE("empty literal", test(UR"(typeclass {})", typeclass_literal{ { 0, 12 }, {} }, &parse_typeclass_literal));

MAYFLY_ADD_TESTCASE("empty declaration",
    test(UR"(typeclass a {})",
        typeclass_definition{ { { 10, 11 }, { lexer::token_type::identifier, UR"(a)", { 10, 11 } } }, typeclass_literal{ { 0, 14 }, {} } },
        &parse_typeclass_definition));

MAYFLY_ADD_TESTCASE("literal with a function declaration",
    test(UR"(typeclass { function foo() -> int; })",
        typeclass_literal{ { 0, 36 },
            { function_declaration{ { 12, 33 },
                { { 21, 24 }, { lexer::token_type::identifier, UR"(foo)", { 21, 24 } } },
                reaver::none,
                reaver::make_optional(expression{ { 30, 33 },
                    postfix_expression{ { 30, 33 }, { identifier{ { 30, 33 }, { lexer::token_type::identifier, UR"(int)", { 30, 33 } } } } } }) } } },
        &parse_typeclass_literal));

MAYFLY_ADD_TESTCASE("declaration with a function declaration",
    test(UR"(typeclass a { function foo() -> int; })",
        typeclass_definition{ { { 10, 11 }, { lexer::token_type::identifier, UR"(a)", { 10, 11 } } },
            typeclass_literal{ { 0, 38 },
                { function_declaration{ { 14, 35 },
                    { { 23, 26 }, { lexer::token_type::identifier, UR"(foo)", { 23, 26 } } },
                    reaver::none,
                    reaver::make_optional(expression{ { 32, 35 },
                        postfix_expression{ { 32, 35 }, { identifier{ { 32, 35 }, { lexer::token_type::identifier, UR"(int)", { 32, 35 } } } } } }) } } } },
        &parse_typeclass_definition));

MAYFLY_ADD_TESTCASE("literal with a function definition",
    test(UR"(typeclass { function foo() -> int { return 1; } })",
        typeclass_literal{ { 0, 49 },
            { function_definition{ { 12, 47 },
                function_declaration{ { 12, 33 },
                    { { 21, 24 }, { lexer::token_type::identifier, UR"(foo)", { 21, 24 } } },
                    reaver::none,
                    reaver::make_optional(expression{ { 30, 33 },
                        postfix_expression{ { 30, 33 }, { identifier{ { 30, 33 }, { lexer::token_type::identifier, UR"(int)", { 30, 33 } } } } } }) },
                block{ { 34, 47 },
                    { statement{ { 36, 45 },
                        return_expression{ { 36, 44 }, { { 43, 44 }, integer_literal{ { 43, 44 }, { lexer::token_type::integer, UR"(1)", { 43, 44 } } } } } } },
                    reaver::none } } } },
        &parse_typeclass_literal));

MAYFLY_ADD_TESTCASE("declaration with a function definition",
    test(UR"(typeclass a { function foo() -> int { return 1; } })",
        typeclass_definition{ { { 10, 11 }, { lexer::token_type::identifier, UR"(a)", { 10, 11 } } },
            typeclass_literal{ { 0, 51 },
                { function_definition{ { 14, 49 },
                    function_declaration{ { 14, 35 },
                        { { 23, 26 }, { lexer::token_type::identifier, UR"(foo)", { 23, 26 } } },
                        reaver::none,
                        reaver::make_optional(expression{ { 32, 35 },
                            postfix_expression{ { 32, 35 }, { identifier{ { 32, 35 }, { lexer::token_type::identifier, UR"(int)", { 32, 35 } } } } } }) },
                    block{ { 36, 49 },
                        { statement{ { 38, 47 },
                            return_expression{ { 38, 46 },
                                { { 45, 46 }, integer_literal{ { 45, 46 }, { lexer::token_type::integer, UR"(1)", { 45, 46 } } } } } } },
                        reaver::none } } } } },
        &parse_typeclass_definition));

MAYFLY_ADD_TESTCASE("literal with both a function definition and a function declaration",
    test(UR"(typeclass { function foo() -> int; function bar() -> int { return 1; } })",
        typeclass_literal{ { 0, 72 },
            { function_declaration{ { 12, 33 },
                  { { 21, 24 }, { lexer::token_type::identifier, UR"(foo)", { 21, 24 } } },
                  reaver::none,
                  reaver::make_optional(expression{ { 30, 33 },
                      postfix_expression{ { 30, 33 }, { identifier{ { 30, 33 }, { lexer::token_type::identifier, UR"(int)", { 30, 33 } } } } } }) },
                function_definition{ { 35, 70 },
                    function_declaration{ { 35, 56 },
                        { { 44, 47 }, { lexer::token_type::identifier, UR"(bar)", { 44, 47 } } },
                        reaver::none,
                        reaver::make_optional(expression{ { 53, 56 },
                            postfix_expression{ { 53, 56 }, { identifier{ { 53, 56 }, { lexer::token_type::identifier, UR"(int)", { 53, 56 } } } } } }) },
                    block{ { 57, 70 },
                        { statement{ { 59, 68 },
                            return_expression{ { 59, 67 },
                                { { 66, 67 }, integer_literal{ { 66, 67 }, { lexer::token_type::integer, UR"(1)", { 66, 67 } } } } } } },
                        reaver::none } } } },
        &parse_typeclass_literal));

MAYFLY_ADD_TESTCASE("declaration with both a function definition and a function declaration",
    test(UR"(typeclass a { function foo() -> int; function bar() -> int { return 1; } })",
        typeclass_definition{ { { 10, 11 }, { lexer::token_type::identifier, UR"(a)", { 10, 11 } } },
            typeclass_literal{ { 0, 74 },
                { function_declaration{ { 14, 35 },
                      { { 23, 26 }, { lexer::token_type::identifier, UR"(foo)", { 23, 26 } } },
                      reaver::none,
                      reaver::make_optional(expression{ { 32, 35 },
                          postfix_expression{ { 32, 35 }, { identifier{ { 32, 35 }, { lexer::token_type::identifier, UR"(int)", { 32, 35 } } } } } }) },
                    function_definition{ { 37, 72 },
                        function_declaration{ { 37, 58 },
                            { { 46, 49 }, { lexer::token_type::identifier, UR"(bar)", { 46, 49 } } },
                            reaver::none,
                            reaver::make_optional(expression{ { 55, 58 },
                                postfix_expression{ { 55, 58 }, { identifier{ { 55, 58 }, { lexer::token_type::identifier, UR"(int)", { 55, 58 } } } } } }) },
                        block{ { 59, 72 },
                            { statement{ { 61, 70 },
                                return_expression{ { 61, 69 },
                                    { { 68, 69 }, integer_literal{ { 68, 69 }, { lexer::token_type::integer, UR"(1)", { 68, 69 } } } } } } },
                            reaver::none } } } } },
        &parse_typeclass_definition));

MAYFLY_END_SUITE;
MAYFLY_END_SUITE;
