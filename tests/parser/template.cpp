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
MAYFLY_BEGIN_SUITE("template");

MAYFLY_BEGIN_SUITE("introducer");

MAYFLY_ADD_TESTCASE("single complete parameter",
    test(UR"(with (T : type))",
        template_introducer{ { 0, 15 },
            { { 6, 14 },
                { parameter{ { 6, 14 },
                    { { 6, 7 }, { lexer::token_type::identifier, UR"(T)", { 6, 7 } } },
                    expression{ { 10, 14 },
                        postfix_expression{ { 10, 14 },
                            identifier{ { 10, 14 }, { lexer::token_type::identifier, UR"(type)", { 10, 14 } } },
                            std::nullopt,
                            {} } } } } } },
        &parse_template_introducer));

MAYFLY_END_SUITE;

MAYFLY_END_SUITE;
MAYFLY_END_SUITE;
