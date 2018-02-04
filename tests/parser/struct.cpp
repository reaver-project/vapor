/**
 * Vapor Compiler Licence
 *
 * Copyright © 2016-2018 Michał "Griwes" Dominiak
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
MAYFLY_BEGIN_SUITE("struct");

MAYFLY_ADD_TESTCASE("empty literal", test(UR"(struct {};)", struct_literal{ { 0, 9 }, {} }, &parse_struct_literal));

MAYFLY_ADD_TESTCASE("single member literal",
    test(UR"(struct { let i : int; };)",
        struct_literal{ { 0, 23 },
            { { declaration{ { 9, 20 },
                std::nullopt,
                { { 13, 14 }, { lexer::token_type::identifier, UR"(i)", { 13, 14 } } },
                std::make_optional<expression>({ { 17, 20 },
                    postfix_expression{ { 17, 20 }, identifier{ { 17, 20 }, { lexer::token_type::identifier, UR"(int)", { 17, 20 } } }, std::nullopt, {} } }),
                std::nullopt } } } },
        &parse_struct_literal));

MAYFLY_ADD_TESTCASE("multiple member literal",
    test(UR"(struct { let i : int; let j = 1; };)",
        struct_literal{ { 0, 34 },
            { { declaration{ { 9, 20 },
                  std::nullopt,
                  { { 13, 14 }, { lexer::token_type::identifier, UR"(i)", { 13, 14 } } },
                  std::make_optional<expression>({ { 17, 20 },
                      postfix_expression{ { 17, 20 }, identifier{ { 17, 20 }, { lexer::token_type::identifier, UR"(int)", { 17, 20 } } }, std::nullopt, {} } }),
                  std::nullopt } },
                { declaration{ { 22, 31 },
                    std::nullopt,
                    { { 26, 27 }, { lexer::token_type::identifier, UR"(j)", { 26, 27 } } },
                    std::nullopt,
                    std::make_optional<expression>(
                        { { 30, 31 }, integer_literal{ { 30, 31 }, { lexer::token_type::integer, UR"(1)", { 30, 31 } }, {} } }) } } } },
        &parse_struct_literal));

MAYFLY_ADD_TESTCASE("empty declaration",
    test(UR"(struct foo {};)",
        declaration{ { 0, 13 },
            std::nullopt,
            { { 7, 10 }, { lexer::token_type::identifier, UR"(foo)", { 7, 10 } } },
            std::nullopt,
            std::make_optional<expression>({ { 0, 13 }, struct_literal{ { 0, 13 }, {} } }) },
        &parse_struct_declaration));

MAYFLY_ADD_TESTCASE("single member declaration",
    test(UR"(struct foo { let i : int; };)",
        declaration{ { 0, 27 },
            std::nullopt,
            { { 7, 10 }, { lexer::token_type::identifier, UR"(foo)", { 7, 10 } } },
            std::nullopt,
            std::make_optional<expression>({ { 0, 27 },
                struct_literal{ { 0, 27 },
                    { { declaration{ { 13, 24 },
                        std::nullopt,
                        { { 17, 18 }, { lexer::token_type::identifier, UR"(i)", { 17, 18 } } },
                        std::make_optional<expression>({ { 21, 24 },
                            postfix_expression{ { 21, 24 },
                                identifier{ { 21, 24 }, { lexer::token_type::identifier, UR"(int)", { 21, 24 } } },
                                std::nullopt,
                                {} } }),
                        std::nullopt } } } } }) },
        &parse_struct_declaration));

MAYFLY_ADD_TESTCASE("multiple member declaration",
    test(UR"(struct foo { let i : int; let j = 1; };)",
        declaration{ { 0, 38 },
            std::nullopt,
            { { 7, 10 }, { lexer::token_type::identifier, UR"(foo)", { 7, 10 } } },
            std::nullopt,
            std::make_optional<expression>({ { 0, 38 },
                struct_literal{ { 0, 38 },
                    { { declaration{ { 13, 24 },
                          std::nullopt,
                          { { 17, 18 }, { lexer::token_type::identifier, UR"(i)", { 17, 18 } } },
                          std::make_optional<expression>({ { 21, 24 },
                              postfix_expression{ { 21, 24 },
                                  identifier{ { 21, 24 }, { lexer::token_type::identifier, UR"(int)", { 21, 24 } } },
                                  std::nullopt,
                                  {} } }),
                          std::nullopt } },
                        { declaration{ { 26, 35 },
                            std::nullopt,
                            { { 30, 31 }, { lexer::token_type::identifier, UR"(j)", { 30, 31 } } },
                            std::nullopt,
                            std::make_optional<expression>(
                                { { 34, 35 }, integer_literal{ { 34, 35 }, { lexer::token_type::integer, UR"(1)", { 34, 35 } }, {} } }) } } } } }) },
        &parse_struct_declaration));

MAYFLY_END_SUITE;
MAYFLY_END_SUITE;
