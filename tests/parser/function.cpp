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
MAYFLY_BEGIN_SUITE("function");

MAYFLY_ADD_TESTCASE("no arguments, deduced type, simple body", test(
    UR"(function foo() => constant;)",
    function{
        { 0, 26 },
        { lexer::token_type::identifier, UR"(foo)", { 9, 12 } },
        {},
        {},
        block{
            { 15, 26 },
            {},
            { expression_list{
                { 18, 26 },
                {
                    {
                        { 18, 26 },
                        postfix_expression{
                            { 18, 26 },
                            { id_expression{
                                { 18, 26 },
                                {
                                    { lexer::token_type::identifier, UR"(constant)", { 18, 26 } }
                                }
                            } },
                            {},
                            {}
                        }
                    }
                }
            } }
        }
    },
    &parse_function
));

MAYFLY_ADD_TESTCASE("no arguments, deduced type, regular body", test(
    UR"(function foo() { => constant })",
    function{
        { 0, 30 },
        { lexer::token_type::identifier, UR"(foo)", { 9, 12 } },
        {},
        {},
        block{
            { 15, 30 },
            {},
            { expression_list{
                { 20, 28 },
                {
                    {
                        { 20, 28 },
                        postfix_expression{
                            { 20, 28 },
                            { id_expression{
                                { 20, 28 },
                                {
                                    { lexer::token_type::identifier, UR"(constant)", { 20, 28 } }
                                }
                            } },
                            {},
                            {}
                        }
                    }
                }
            } }
        }
    },
    &parse_function
));

MAYFLY_END_SUITE;
MAYFLY_END_SUITE;

