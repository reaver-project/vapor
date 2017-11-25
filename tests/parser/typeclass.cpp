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

MAYFLY_BEGIN_SUITE("definition");

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
                std::nullopt,
                std::make_optional(expression{ { 30, 33 },
                    postfix_expression{ { 30, 33 }, { identifier{ { 30, 33 }, { lexer::token_type::identifier, UR"(int)", { 30, 33 } } } } } }) } } },
        &parse_typeclass_literal));

MAYFLY_ADD_TESTCASE("declaration with a function declaration",
    test(UR"(typeclass a { function foo() -> int; })",
        typeclass_definition{ { { 10, 11 }, { lexer::token_type::identifier, UR"(a)", { 10, 11 } } },
            typeclass_literal{ { 0, 38 },
                { function_declaration{ { 14, 35 },
                    { { 23, 26 }, { lexer::token_type::identifier, UR"(foo)", { 23, 26 } } },
                    std::nullopt,
                    std::make_optional(expression{ { 32, 35 },
                        postfix_expression{ { 32, 35 }, { identifier{ { 32, 35 }, { lexer::token_type::identifier, UR"(int)", { 32, 35 } } } } } }) } } } },
        &parse_typeclass_definition));

MAYFLY_ADD_TESTCASE("literal with a function definition",
    test(UR"(typeclass { function foo() -> int { return 1; } })",
        typeclass_literal{ { 0, 49 },
            { function_definition{ { 12, 47 },
                function_declaration{ { 12, 33 },
                    { { 21, 24 }, { lexer::token_type::identifier, UR"(foo)", { 21, 24 } } },
                    std::nullopt,
                    std::make_optional(expression{ { 30, 33 },
                        postfix_expression{ { 30, 33 }, { identifier{ { 30, 33 }, { lexer::token_type::identifier, UR"(int)", { 30, 33 } } } } } }) },
                block{ { 34, 47 },
                    { statement{ { 36, 45 },
                        return_expression{ { 36, 44 }, { { 43, 44 }, integer_literal{ { 43, 44 }, { lexer::token_type::integer, UR"(1)", { 43, 44 } } } } } } },
                    std::nullopt } } } },
        &parse_typeclass_literal));

MAYFLY_ADD_TESTCASE("declaration with a function definition",
    test(UR"(typeclass a { function foo() -> int { return 1; } })",
        typeclass_definition{ { { 10, 11 }, { lexer::token_type::identifier, UR"(a)", { 10, 11 } } },
            typeclass_literal{ { 0, 51 },
                { function_definition{ { 14, 49 },
                    function_declaration{ { 14, 35 },
                        { { 23, 26 }, { lexer::token_type::identifier, UR"(foo)", { 23, 26 } } },
                        std::nullopt,
                        std::make_optional(expression{ { 32, 35 },
                            postfix_expression{ { 32, 35 }, { identifier{ { 32, 35 }, { lexer::token_type::identifier, UR"(int)", { 32, 35 } } } } } }) },
                    block{ { 36, 49 },
                        { statement{ { 38, 47 },
                            return_expression{ { 38, 46 },
                                { { 45, 46 }, integer_literal{ { 45, 46 }, { lexer::token_type::integer, UR"(1)", { 45, 46 } } } } } } },
                        std::nullopt } } } } },
        &parse_typeclass_definition));

MAYFLY_ADD_TESTCASE("literal with both a function definition and a function declaration",
    test(UR"(typeclass { function foo() -> int; function bar() -> int { return 1; } })",
        typeclass_literal{ { 0, 72 },
            { function_declaration{ { 12, 33 },
                  { { 21, 24 }, { lexer::token_type::identifier, UR"(foo)", { 21, 24 } } },
                  std::nullopt,
                  std::make_optional(expression{ { 30, 33 },
                      postfix_expression{ { 30, 33 }, { identifier{ { 30, 33 }, { lexer::token_type::identifier, UR"(int)", { 30, 33 } } } } } }) },
                function_definition{ { 35, 70 },
                    function_declaration{ { 35, 56 },
                        { { 44, 47 }, { lexer::token_type::identifier, UR"(bar)", { 44, 47 } } },
                        std::nullopt,
                        std::make_optional(expression{ { 53, 56 },
                            postfix_expression{ { 53, 56 }, { identifier{ { 53, 56 }, { lexer::token_type::identifier, UR"(int)", { 53, 56 } } } } } }) },
                    block{ { 57, 70 },
                        { statement{ { 59, 68 },
                            return_expression{ { 59, 67 },
                                { { 66, 67 }, integer_literal{ { 66, 67 }, { lexer::token_type::integer, UR"(1)", { 66, 67 } } } } } } },
                        std::nullopt } } } },
        &parse_typeclass_literal));

MAYFLY_ADD_TESTCASE("declaration with both a function definition and a function declaration",
    test(UR"(typeclass a { function foo() -> int; function bar() -> int { return 1; } })",
        typeclass_definition{ { { 10, 11 }, { lexer::token_type::identifier, UR"(a)", { 10, 11 } } },
            typeclass_literal{ { 0, 74 },
                { function_declaration{ { 14, 35 },
                      { { 23, 26 }, { lexer::token_type::identifier, UR"(foo)", { 23, 26 } } },
                      std::nullopt,
                      std::make_optional(expression{ { 32, 35 },
                          postfix_expression{ { 32, 35 }, { identifier{ { 32, 35 }, { lexer::token_type::identifier, UR"(int)", { 32, 35 } } } } } }) },
                    function_definition{ { 37, 72 },
                        function_declaration{ { 37, 58 },
                            { { 46, 49 }, { lexer::token_type::identifier, UR"(bar)", { 46, 49 } } },
                            std::nullopt,
                            std::make_optional(expression{ { 55, 58 },
                                postfix_expression{ { 55, 58 }, { identifier{ { 55, 58 }, { lexer::token_type::identifier, UR"(int)", { 55, 58 } } } } } }) },
                        block{ { 59, 72 },
                            { statement{ { 61, 70 },
                                return_expression{ { 61, 69 },
                                    { { 68, 69 }, integer_literal{ { 68, 69 }, { lexer::token_type::integer, UR"(1)", { 68, 69 } } } } } } },
                            std::nullopt } } } } },
        &parse_typeclass_definition));

MAYFLY_END_SUITE;

MAYFLY_BEGIN_SUITE("instance");

MAYFLY_ADD_TESTCASE("default instance with no members",
    test(UR"(default instance a(int) {})",
        default_instance_definition{ { 0, 26 },
            instance_literal{ { 8, 26 },
                { { 17, 18 }, { { { 17, 18 }, { lexer::token_type::identifier, UR"(a)", { 17, 18 } } } } },
                { { 19, 22 },
                    { expression{ { 19, 22 },
                        postfix_expression{ { 19, 22 }, { identifier{ { 19, 22 }, { lexer::token_type::identifier, UR"(int)", { 19, 22 } } } } } } } },
                {} } },
        &parse_default_instance));

MAYFLY_ADD_TESTCASE("instance literal with no members",
    test(UR"(instance a(int) {})",
        instance_literal{ { 0, 18 },
            { { 9, 10 }, { { { 9, 10 }, { lexer::token_type::identifier, UR"(a)", { 9, 10 } } } } },
            { { 11, 14 },
                { expression{ { 11, 14 },
                    postfix_expression{ { 11, 14 }, { identifier{ { 11, 14 }, { lexer::token_type::identifier, UR"(int)", { 11, 14 } } } } } } } },
            {} },
        &parse_instance_literal));

MAYFLY_ADD_TESTCASE("default instance with definitions",
    test(UR"(default instance a(int) { function foo(arg : int) {} function bar(arg) {} })",
        default_instance_definition{ { 0, 75 },
            instance_literal{ { 8, 75 },
                { { 17, 18 }, { { { 17, 18 }, { lexer::token_type::identifier, UR"(a)", { 17, 18 } } } } },
                { { 19, 22 },
                    { expression{ { 19, 22 },
                        postfix_expression{ { 19, 22 }, { identifier{ { 19, 22 }, { lexer::token_type::identifier, UR"(int)", { 19, 22 } } } } } } } },
                { function_definition{ { 26, 52 },
                      function_declaration{ { 26, 49 },
                          { { 35, 38 }, { lexer::token_type::identifier, UR"(foo)", { 35, 38 } } },
                          parameter_list{ { 39, 48 },
                              { parameter{ { 39, 48 },
                                  { { 39, 42 }, { lexer::token_type::identifier, UR"(arg)", { 39, 42 } } },
                                  std::make_optional(expression{ { 45, 48 },
                                      postfix_expression{ { 45, 48 },
                                          identifier{ { 45, 48 }, { lexer::token_type::identifier, UR"(int)", { 45, 48 } } } } }) } } },
                          std::nullopt },
                      block{ { 50, 52 }, {}, std::nullopt } },
                    function_definition{ { 53, 73 },
                        function_declaration{ { 53, 70 },
                            { { 62, 65 }, { lexer::token_type::identifier, UR"(bar)", { 62, 65 } } },
                            parameter_list{ { 66, 69 },
                                { parameter{ { 66, 69 }, { { 66, 69 }, { lexer::token_type::identifier, UR"(arg)", { 66, 69 } } }, std::nullopt } } },
                            std::nullopt },
                        block{ { 71, 73 }, {}, std::nullopt } } } } },
        &parse_default_instance));

MAYFLY_ADD_TESTCASE("instance literal with definitions",
    test(UR"(instance a(int) { function foo(arg : int) {} function bar(arg) {} })",
        instance_literal{ { 0, 67 },
            { { 9, 10 }, { { { 9, 10 }, { lexer::token_type::identifier, UR"(a)", { 9, 10 } } } } },
            { { 11, 14 },
                { expression{ { 11, 14 },
                    postfix_expression{ { 11, 14 }, { identifier{ { 11, 14 }, { lexer::token_type::identifier, UR"(int)", { 11, 14 } } } } } } } },
            { function_definition{ { 18, 44 },
                  function_declaration{ { 18, 41 },
                      { { 27, 30 }, { lexer::token_type::identifier, UR"(foo)", { 27, 30 } } },
                      parameter_list{ { 31, 40 },
                          { parameter{ { 31, 40 },
                              { { 31, 34 }, { lexer::token_type::identifier, UR"(arg)", { 31, 34 } } },
                              std::make_optional(expression{ { 37, 40 },
                                  postfix_expression{ { 37, 40 }, identifier{ { 37, 40 }, { lexer::token_type::identifier, UR"(int)", { 37, 40 } } } } }) } } },
                      std::nullopt },
                  block{ { 42, 44 }, {}, std::nullopt } },
                function_definition{ { 45, 65 },
                    function_declaration{ { 45, 62 },
                        { { 54, 57 }, { lexer::token_type::identifier, UR"(bar)", { 54, 57 } } },
                        parameter_list{ { 58, 61 },
                            { parameter{ { 58, 61 }, { { 58, 61 }, { lexer::token_type::identifier, UR"(arg)", { 58, 61 } } }, std::nullopt } } },
                        std::nullopt },
                    block{ { 63, 65 }, {}, std::nullopt } } } },
        &parse_instance_literal));

MAYFLY_END_SUITE;

MAYFLY_END_SUITE;
MAYFLY_END_SUITE;
