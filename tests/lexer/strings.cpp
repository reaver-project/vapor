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

#include <reaver/mayfly.h>

#include <string>
#include <vector>

#include "vapor/lexer.h"
#include "helpers.h"

using namespace reaver::vapor::lexer;

MAYFLY_BEGIN_SUITE("lexer");
MAYFLY_BEGIN_SUITE("strings");

MAYFLY_ADD_TESTCASE("simple string", test(R"("foo" "bar")",
    {
        { token_type::string, "\"foo\"", { 0, 5 } },
        { token_type::string, "\"bar\"", { 6, 11 } }
    }
));

MAYFLY_ADD_TESTCASE("escaped string", test(R"("foo\"bar")",
    {
        { token_type::string, "\"foo\\\"bar\"", { 0, 10 } },
    }
));

MAYFLY_ADD_TESTCASE("line broken string", test(R"("foo\
bar")",
    {
        { token_type::string, "\"foo\\\nbar\"", { 0, 10 } },
    }
));

MAYFLY_END_SUITE;
MAYFLY_END_SUITE;
