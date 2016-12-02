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

#include <reaver/mayfly.h>

#include "helpers.h"

using namespace reaver::vapor;
using namespace reaver::vapor::parser;

MAYFLY_BEGIN_SUITE("parser");
MAYFLY_BEGIN_SUITE("declaration");

MAYFLY_ADD_TESTCASE("with deduced type",
    test(UR"(let foo = 1;)",
        declaration{ { 0, 11 },
            { lexer::token_type::identifier, UR"(foo)", { 4, 7 } },
            reaver::none,
            reaver::make_optional<expression>({ { 10, 11 }, integer_literal{ { 10, 11 }, { lexer::token_type::integer, UR"(1)", { 10, 11 } }, {} } }) },
        [](auto && ctx) { return parse_declaration(ctx); }));

MAYFLY_ADD_TESTCASE("with explicit type",
    test(UR"(let foo : int = 1;)",
        declaration{ { 0, 17 },
            { lexer::token_type::identifier, UR"(foo)", { 4, 7 } },
            reaver::make_optional<expression>({ { 10, 13 },
                postfix_expression{ { 10, 13 },
                    id_expression{ { 10, 13 }, { { lexer::token_type::identifier, UR"(int)", { 10, 13 } } } },
                    reaver::none,
                    {} } }),
            reaver::make_optional<expression>({ { 16, 17 }, integer_literal{ { 16, 17 }, { lexer::token_type::integer, UR"(1)", { 16, 17 } }, {} } }) },
        [](auto && ctx) { return parse_declaration(ctx); }));

MAYFLY_ADD_TESTCASE("member with deduced type",
    test(UR"(let foo = 1;)",
        declaration{ { 0, 11 },
            { lexer::token_type::identifier, UR"(foo)", { 4, 7 } },
            reaver::none,
            reaver::make_optional<expression>({ { 10, 11 }, integer_literal{ { 10, 11 }, { lexer::token_type::integer, UR"(1)", { 10, 11 } }, {} } }) },
        [](auto && ctx) { return parse_declaration(ctx, declaration_mode::member_declaration); }));

MAYFLY_ADD_TESTCASE("member with explicit type",
    test(UR"(let foo : int = 1;)",
        declaration{ { 0, 17 },
            { lexer::token_type::identifier, UR"(foo)", { 4, 7 } },
            reaver::make_optional<expression>({ { 10, 13 },
                postfix_expression{ { 10, 13 },
                    id_expression{ { 10, 13 }, { { lexer::token_type::identifier, UR"(int)", { 10, 13 } } } },
                    reaver::none,
                    {} } }),
            reaver::make_optional<expression>({ { 16, 17 }, integer_literal{ { 16, 17 }, { lexer::token_type::integer, UR"(1)", { 16, 17 } }, {} } }) },
        [](auto && ctx) { return parse_declaration(ctx, declaration_mode::member_declaration); }));

MAYFLY_ADD_TESTCASE("member without initializer",
    test(UR"(let foo : int;)",
        declaration{ { 0, 13 },
            { lexer::token_type::identifier, UR"(foo)", { 4, 7 } },
            reaver::make_optional<expression>({ { 10, 13 },
                postfix_expression{ { 10, 13 },
                    id_expression{ { 10, 13 }, { { lexer::token_type::identifier, UR"(int)", { 10, 13 } } } },
                    reaver::none,
                    {} } }),
            reaver::none },
        [](auto && ctx) { return parse_declaration(ctx, declaration_mode::member_declaration); }));

MAYFLY_END_SUITE;
MAYFLY_END_SUITE;
