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
MAYFLY_BEGIN_SUITE("lambda_expression");

MAYFLY_ADD_TESTCASE("no arguments, deduced type, simple body", test(
    UR"(λ => constant;)",
    lambda_expression{
        { 0, 13 },
        {},
        {},
        block{
            { 2, 13 },
            {},
            { expression_list{
                { 5, 13 },
                {
                    {
                        { 5, 13 },
                        postfix_expression{
                            { 5, 13 },
                            { id_expression{
                                { 5, 13 },
                                {
                                    { lexer::token_type::identifier, UR"(constant)", { 5, 13 } }
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
    &parse_lambda_expression
));

MAYFLY_ADD_TESTCASE("no arguments, deduced type, regular body", test(
    UR"(λ { => constant })",
    lambda_expression{
        { 0, 17 },
        {},
        {},
        block{
            { 2, 17 },
            {},
            { expression_list{
                { 7, 15 },
                {
                    {
                        { 7, 15 },
                        postfix_expression{
                            { 7, 15 },
                            { id_expression{
                                { 7, 15 },
                                {
                                    { lexer::token_type::identifier, UR"(constant)", { 7, 15 } }
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
    &parse_lambda_expression
));

MAYFLY_END_SUITE;
MAYFLY_END_SUITE;

