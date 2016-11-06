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

#include "vapor/parser/declaration.h"
#include "vapor/parser/lambda_expression.h"

reaver::vapor::parser::_v1::declaration reaver::vapor::parser::_v1::parse_declaration(reaver::vapor::parser::_v1::context & ctx)
{
    declaration ret;

    auto start = expect(ctx, lexer::token_type::let).range.start();
    ret.identifier = expect(ctx, lexer::token_type::identifier);

    if (peek(ctx) && peek(ctx)->type == lexer::token_type::colon)
    {
        expect(ctx, lexer::token_type::colon);

        ret.type_expression = parse_expression(ctx, expression_special_modes::assignment);
    }

    expect(ctx, lexer::token_type::assign);
    ret.rhs = parse_expression(ctx);
    ret.range = { start, ret.rhs.range.end() };

    return ret;
}

void reaver::vapor::parser::_v1::print(const reaver::vapor::parser::_v1::declaration & decl, std::ostream & os, std::size_t indent)
{
    auto in = std::string(indent, ' ');

    os << in << "`declaration` at " << decl.range << '\n';

    os << in << "{\n";
    print(decl.identifier, os, indent + 4);
    print(decl.rhs, os, indent + 4);
    os << in << "}\n";
}

