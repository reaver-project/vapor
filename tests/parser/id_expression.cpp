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
MAYFLY_BEGIN_SUITE("id_expression");

MAYFLY_ADD_TESTCASE("simple id-expression",
    test(UR"(foo)", id_expression{ { 0, 3 }, { identifier{ { 0, 3 }, { lexer::token_type::identifier, UR"(foo)", { 0, 3 } } } } }, &parse_id_expression));

MAYFLY_ADD_TESTCASE("complex id-expression",
    test(UR"(foo.bar.baz)",
        id_expression{ { 0, 11 },
            { identifier{ { 0, 3 }, { lexer::token_type::identifier, UR"(foo)", { 0, 3 } } },
                identifier{ { 4, 7 }, { lexer::token_type::identifier, UR"(bar)", { 4, 7 } } },
                identifier{ { 8, 11 }, { lexer::token_type::identifier, UR"(baz)", { 8, 11 } } } } },
        &parse_id_expression));

MAYFLY_ADD_TESTCASE("invalid id-expression", test(UR"(.foo.bar)", 0, [](auto && ctx) {
    MAYFLY_REQUIRE_THROWS_TYPE(expectation_failure, parse_id_expression(ctx));
    return 0;
}));

MAYFLY_END_SUITE;
MAYFLY_END_SUITE;
