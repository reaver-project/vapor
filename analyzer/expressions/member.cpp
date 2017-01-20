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

#include "vapor/analyzer/expressions/member.h"
#include "vapor/analyzer/symbol.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    member_expression::member_expression(const parser::member_expression & parse) : _parse{ parse }
    {
    }

    void member_expression::print(std::ostream & os, std::size_t indent) const
    {
        assert(0);
    }

    variable * member_expression::get_variable() const
    {
        if (!_referenced)
        {
            return expression::get_variable();
        }

        return _referenced;
    }

    statement_ir member_expression::_codegen_ir(ir_generation_context & ctx) const
    {
        assert(0);
    }
}
}
