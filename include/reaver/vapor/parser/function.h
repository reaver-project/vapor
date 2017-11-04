/**
 * Vapor Compiler Licence
 *
 * Copyright © 2015-2017 Michał "Griwes" Dominiak
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

#pragma once

#include <reaver/optional.h>

#include "../range.h"
#include "expression.h"
#include "helpers.h"
#include "parameter_list.h"

namespace reaver::vapor::parser
{
inline namespace _v1
{
    struct block;

    struct function_declaration
    {
        range_type range;
        identifier name;
        optional<parameter_list> parameters;
        optional<expression> return_type;
    };

    struct function_definition
    {
        range_type range;
        function_declaration signature;
        recursive_wrapper<block> body;
    };

    bool operator==(const function_declaration & lhs, const function_declaration & rhs);
    bool operator==(const function_definition & lhs, const function_definition & rhs);

    function_declaration parse_function_declaration(context & ctx);
    function_definition parse_function_definition(context & ctx, optional<function_declaration> decl = none);

    void print(const function_declaration & f, std::ostream & os, print_context ctx);
    void print(const function_definition & f, std::ostream & os, print_context ctx);

    variant<function_declaration, function_definition> parse_function(context & ctx);
}
}
