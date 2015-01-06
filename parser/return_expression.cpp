/**
 * Vapor Compiler Licence
 *
 * Copyright © 2015 Michał "Griwes" Dominiak
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

#include "vapor/parser/return_expression.h"
#include "vapor/parser/expression_list.h"
#include "vapor/parser/lambda_expression.h"

reaver::vapor::parser::_v1::return_expression reaver::vapor::parser::_v1::parse_return_expression(reaver::vapor::parser::_v1::context & ctx)
{
    return_expression ret;

    auto start = expect(ctx, lexer::token_type::return_).range.start();
    ret.return_value = parse_expression(ctx);

    ret.range = { start, ret.return_value.range.end() };

    return ret;
}

void reaver::vapor::parser::_v1::print(const reaver::vapor::parser::_v1::return_expression & ret, std::ostream & os, std::size_t indent)
{
    auto in = std::string(indent, ' ');

    os << in << "`return-expression` at " << ret.range << '\n';
    os << in << "{\n";
    print(ret.return_value, os, indent + 4);
    os << in << "}\n";
}
