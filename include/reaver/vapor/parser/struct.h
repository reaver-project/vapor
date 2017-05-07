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

#pragma once

#include <reaver/variant.h>

#include "declaration.h"
#include "function.h"

namespace reaver::vapor::parser
{
inline namespace _v1
{
    struct struct_literal
    {
        range_type range;
        std::vector<variant<declaration, function>> members;
    };

    inline bool operator==(const struct_literal & lhs, const struct_literal & rhs)
    {
        return lhs.range == rhs.range && lhs.members == rhs.members;
    }

    struct_literal parse_struct_literal(context & ctx);
    declaration parse_struct_declaration(context & ctx);

    void print(const struct_literal & lit, std::ostream & os, print_context ctx);
}
}
