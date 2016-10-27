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

MAYFLY_ADD_TESTCASE("one argument, deduced type, simple body", test(
    UR"(function foo(x : int) => constant;)",
    function{
        { 0, 33 },
        { lexer::token_type::identifier, UR"(foo)", { 9, 12 } },
        reaver::make_optional(argument_list{
            { 13, 20 },
            {
                argument{
                    { 13, 20 },
                    { lexer::token_type::identifier, UR"(x)", { 13, 14 } },
                    {
                        { 17, 20 },
                        postfix_expression{
                            { 17, 20 },
                            id_expression{
                                { 17, 20 },
                                {
                                    { lexer::token_type::identifier, UR"(int)", { 17, 20 } }
                                }
                            },
                            {},
                            {}
                        }
                    }
                }
            }
        }),
        {},
        block{
            { 22, 33 },
            {},
            { expression_list{
                { 25, 33 },
                {
                    {
                        { 25, 33 },
                        postfix_expression{
                            { 25, 33 },
                            { id_expression{
                                { 25, 33 },
                                {
                                    { lexer::token_type::identifier, UR"(constant)", { 25, 33 } }
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

MAYFLY_ADD_TESTCASE("two arguments, deduced type, simple body", test(
    UR"(function foo(x : int, y : bool) => constant;)",
    function{
        { 0, 43 },
        { lexer::token_type::identifier, UR"(foo)", { 9, 12 } },
        reaver::make_optional(argument_list{
            { 13, 30 },
            {
                argument{
                    { 13, 20 },
                    { lexer::token_type::identifier, UR"(x)", { 13, 14 } },
                    {
                        { 17, 20 },
                        postfix_expression{
                            { 17, 20 },
                            id_expression{
                                { 17, 20 },
                                {
                                    { lexer::token_type::identifier, UR"(int)", { 17, 20 } }
                                }
                            },
                            {},
                            {}
                        }
                    }
                },
                argument{
                    { 22, 30 },
                    { lexer::token_type::identifier, UR"(y)", { 22, 23 } },
                    {
                        { 26, 30 },
                        postfix_expression{
                            { 26, 30 },
                            id_expression{
                                { 26, 30 },
                                {
                                    { lexer::token_type::identifier, UR"(bool)", { 26, 30 } }
                                }
                            },
                            {},
                            {}
                        }
                    }
                }
            }
        }),
        {},
        block{
            { 32, 43 },
            {},
            { expression_list{
                { 35, 43 },
                {
                    {
                        { 35, 43 },
                        postfix_expression{
                            { 35, 43 },
                            { id_expression{
                                { 35, 43 },
                                {
                                    { lexer::token_type::identifier, UR"(constant)", { 35, 43 } }
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

