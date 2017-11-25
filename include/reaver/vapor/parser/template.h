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

#pragma once

#include "declaration.h"
#include "helpers.h"
#include "parameter_list.h"
#include "typeclass.h"

namespace reaver::vapor::parser
{
inline namespace _v1
{
    struct template_introducer
    {
        range_type range;
        parameter_list template_parameters;
    };

    struct template_expression
    {
        range_type range;
        template_introducer parameters;
        std::variant<typeclass_literal> expression = typeclass_literal{};
    };

    bool operator==(const template_introducer & lhs, const template_introducer & rhs);
    bool operator==(const template_expression & lhs, const template_expression & rhs);

    template_introducer parse_template_introducer(context & ctx);
    template_expression parse_template_expression(context & ctx);
    declaration parse_template_declaration(context & ctx);

    void print(const template_introducer & tpl, std::ostream & os, print_context ctx);
    void print(const template_expression & tpl, std::ostream & os, print_context ctx);
}
}
