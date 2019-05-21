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
MAYFLY_BEGIN_SUITE("declaration");

MAYFLY_ADD_TESTCASE("with deduced type",
    test(UR"(let foo = 1;)",
        declaration{ { 0, 11 },
            std::nullopt,
            { { 4, 7 }, { lexer::token_type::identifier, UR"(foo)", { 4, 7 } } },
            std::nullopt,
            std::make_optional<expression>({ { 10, 11 },
                integer_literal{ { 10, 11 }, { lexer::token_type::integer, UR"(1)", { 10, 11 } }, {} } }) },
        [](auto && ctx) { return parse_declaration(ctx); }));

MAYFLY_ADD_TESTCASE("with explicit type",
    test(UR"(let foo : int = 1;)",
        declaration{ { 0, 17 },
            std::nullopt,
            { { 4, 7 }, { lexer::token_type::identifier, UR"(foo)", { 4, 7 } } },
            std::make_optional<expression>({ { 10, 13 },
                postfix_expression{ { 10, 13 },
                    identifier{ { 10, 13 }, { lexer::token_type::identifier, UR"(int)", { 10, 13 } } },
                    std::nullopt,
                    {} } }),
            std::make_optional<expression>({ { 16, 17 },
                integer_literal{ { 16, 17 }, { lexer::token_type::integer, UR"(1)", { 16, 17 } }, {} } }) },
        [](auto && ctx) { return parse_declaration(ctx); }));

MAYFLY_ADD_TESTCASE("member with deduced type",
    test(UR"(let foo = 1;)",
        declaration{ { 0, 11 },
            std::nullopt,
            { { 4, 7 }, { lexer::token_type::identifier, UR"(foo)", { 4, 7 } } },
            std::nullopt,
            std::make_optional<expression>({ { 10, 11 },
                integer_literal{ { 10, 11 }, { lexer::token_type::integer, UR"(1)", { 10, 11 } }, {} } }) },
        [](auto && ctx) { return parse_declaration(ctx, declaration_mode::member); }));

MAYFLY_ADD_TESTCASE("member with explicit type",
    test(UR"(let foo : int = 1;)",
        declaration{ { 0, 17 },
            std::nullopt,
            { { 4, 7 }, { lexer::token_type::identifier, UR"(foo)", { 4, 7 } } },
            std::make_optional<expression>({ { 10, 13 },
                postfix_expression{ { 10, 13 },
                    identifier{ { 10, 13 }, { lexer::token_type::identifier, UR"(int)", { 10, 13 } } },
                    std::nullopt,
                    {} } }),
            std::make_optional<expression>({ { 16, 17 },
                integer_literal{ { 16, 17 }, { lexer::token_type::integer, UR"(1)", { 16, 17 } }, {} } }) },
        [](auto && ctx) { return parse_declaration(ctx, declaration_mode::member); }));

MAYFLY_ADD_TESTCASE("member without initializer",
    test(UR"(let foo : int;)",
        declaration{ { 0, 13 },
            std::nullopt,
            { { 4, 7 }, { lexer::token_type::identifier, UR"(foo)", { 4, 7 } } },
            std::make_optional<expression>({ { 10, 13 },
                postfix_expression{ { 10, 13 },
                    identifier{ { 10, 13 }, { lexer::token_type::identifier, UR"(int)", { 10, 13 } } },
                    std::nullopt,
                    {} } }),
            std::nullopt },
        [](auto && ctx) { return parse_declaration(ctx, declaration_mode::member); }));

MAYFLY_ADD_TESTCASE("with deduced type, exported",
    test(UR"(export let foo = 1;)",
        declaration{ { 0, 18 },
            lexer::token{ lexer::token_type::export_, UR"(export)", { 0, 7 } },
            { { 11, 14 }, { lexer::token_type::identifier, UR"(foo)", { 11, 14 } } },
            std::nullopt,
            std::make_optional<expression>({ { 17, 18 },
                integer_literal{ { 17, 18 }, { lexer::token_type::integer, UR"(1)", { 17, 18 } }, {} } }) },
        [](auto && ctx) { return parse_declaration(ctx, declaration_mode::module_scope); }));

MAYFLY_ADD_TESTCASE("exported, invalid", test(UR"(export let foo = 1;)", 0, [](auto && ctx) {
    auto ctx_orig = ctx;
    MAYFLY_REQUIRE_THROWS_TYPE(expectation_failure, parse_declaration(ctx, declaration_mode::variable));
    MAYFLY_REQUIRE_THROWS_TYPE(expectation_failure, parse_declaration(ctx_orig, declaration_mode::member));
    return 0;
}));

MAYFLY_END_SUITE;
MAYFLY_END_SUITE;
