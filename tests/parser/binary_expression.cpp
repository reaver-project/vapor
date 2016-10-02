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
MAYFLY_BEGIN_SUITE("binary_expression");

MAYFLY_ADD_TESTCASE("simple binary-expression", test(UR"(1 + 2;)",
    expression{
        { 0, 5 },
        binary_expression{
            { 0, 5 },
            { lexer::token_type::plus, UR"(+)", { 2, 3 } },
            {
                { 0, 1 },
                integer_literal{
                    { 0, 1 },
                    { lexer::token_type::integer, UR"(1)", { 0, 1 } },
                    {}
                }
            },
            {
                { 4, 5 },
                integer_literal{
                    { 4, 5 },
                    { lexer::token_type::integer, UR"(2)", { 4, 5 } },
                    {}
                }
            }
        }
    },
    [](auto && ctx) {
        return parse_expression(ctx);
    }
));

MAYFLY_ADD_TESTCASE("equal precedence, left associative", test(UR"(1 + 2 + 3;)",
    expression{
        { 0, 9 },
        binary_expression{
            { 0, 9 },
            { lexer::token_type::plus, UR"(+)", { 6, 7 } },
            {
                { 0, 5 },
                binary_expression{
                    { 0, 5 },
                    { lexer::token_type::plus, UR"(+)", { 2, 3 } },
                    {
                        { 0, 1 },
                        integer_literal{
                            { 0, 1 },
                            { lexer::token_type::integer, UR"(1)", { 0, 1 } },
                            {}
                        }
                    },
                    {
                        { 4, 5 },
                        integer_literal{
                            { 4, 5 },
                            { lexer::token_type::integer, UR"(2)", { 4, 5 } },
                            {}
                        }
                    }
                }
            },
            {
                { 8, 9 },
                integer_literal{
                    { 8, 9 },
                    { lexer::token_type::integer, UR"(3)", { 8, 9 } },
                    {}
                }
            }
        }

    },
    [](auto && ctx) {
        return parse_expression(ctx);
    }
));

MAYFLY_ADD_TESTCASE("equal precedence, right associative", test(UR"(1 = 2 = 3;)",
    expression{
        { 0, 9 },
        binary_expression{
            { 0, 9 },
            { lexer::token_type::assign, UR"(=)", { 2, 3 } },
            {
                { 0, 1 },
                integer_literal{
                    { 0, 1 },
                    { lexer::token_type::integer, UR"(1)", { 0, 1 } },
                    {}
                }
            },
            {
                { 4, 9 },
                binary_expression{
                    { 4, 9 },
                    { lexer::token_type::assign, UR"(=)", { 6, 7 } },
                    {
                        { 4, 5 },
                        integer_literal{
                            { 4, 5 },
                            { lexer::token_type::integer, UR"(2)", { 4, 5 } },
                            {}
                        }
                    },
                    {
                        { 8, 9 },
                        integer_literal{
                            { 8, 9 },
                            { lexer::token_type::integer, UR"(3)", { 8, 9 } },
                            {}
                        }
                    }
                }
            }
        }

    },
    [](auto && ctx) {
        return parse_expression(ctx);
    }
));

MAYFLY_ADD_TESTCASE("lower-higher precedence, left associative", test(UR"(1 + 2 * 3;)",
    expression{
        { 0, 9 },
        binary_expression{
            { 0, 9 },
            { lexer::token_type::plus, UR"(+)", { 2, 3 } },
            {
                { 0, 1 },
                integer_literal{
                    { 0, 1 },
                    { lexer::token_type::integer, UR"(1)", { 0, 1 } },
                    {}
                }
            },
            {
                { 4, 9 },
                binary_expression{
                    { 4, 9 },
                    { lexer::token_type::star, UR"(*)", { 6, 7 } },
                    {
                        { 4, 5 },
                        integer_literal{
                            { 4, 5 },
                            { lexer::token_type::integer, UR"(2)", { 4, 5 } },
                            {}
                        }
                    },
                    {
                        { 8, 9 },
                        integer_literal{
                            { 8, 9 },
                            { lexer::token_type::integer, UR"(3)", { 8, 9 } },
                            {}
                        }
                    }
                }
            }
        }

    },
    [](auto && ctx) {
        return parse_expression(ctx);
    }
));

MAYFLY_ADD_TESTCASE("lower-higher-lower precedence, left associative", test(UR"(1 + 2 * 3 + 4;)",
    expression{
        { 0, 13 },
        binary_expression{
            { 0, 13 },
            { lexer::token_type::plus, UR"(+)", { 10, 11 } },
            {
                { 0, 9 },
                binary_expression{
                    { 0, 9 },
                    { lexer::token_type::plus, UR"(+)", { 2, 3 } },
                    {
                        { 0, 1 },
                        integer_literal{
                            { 0, 1 },
                            { lexer::token_type::integer, UR"(1)", { 0, 1 } },
                            {}
                        }
                    },
                    {
                        { 4, 9 },
                        binary_expression{
                            { 4, 9 },
                            { lexer::token_type::star, UR"(*)", { 6, 7 } },
                            {
                                { 4, 5 },
                                integer_literal{
                                    { 4, 5 },
                                    { lexer::token_type::integer, UR"(2)", { 4, 5 } },
                                    {}
                                }
                            },
                            {
                                { 8, 9 },
                                integer_literal{
                                    { 8, 9 },
                                    { lexer::token_type::integer, UR"(3)", { 8, 9 } },
                                    {}
                                }
                            }
                        }
                    }
                }
            },
            {
                { 12, 13 },
                integer_literal{
                    { 12, 13 },
                    { lexer::token_type::integer, UR"(4)", { 12, 13 } },
                    {}
                }
            }
        }

    },
    [](auto && ctx) {
        return parse_expression(ctx);
    }
));

MAYFLY_END_SUITE;
MAYFLY_END_SUITE;

