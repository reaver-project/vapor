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

#include "vapor/parser.h"
#include "vapor/analyzer/binary_expression.h"
#include "vapor/analyzer/function.h"

void reaver::vapor::analyzer::_v1::binary_expression::print(std::ostream & os, std::size_t indent) const
{
    auto in = std::string(indent, ' ');
    os << in << "binary expression at " << _parse.range << '\n';
    os << in << "type: " << get_variable()->get_type()->explain() << '\n';
    os << in << "selected overload: " << _overload->explain() << '\n';
    os << in << "lhs:\n";
    os << in << "{\n";
    _lhs->print(os, indent + 4);
    os << in << "}\n";

    os << in << "operator: " << lexer::token_types[+_op.type] << '\n';

    os << in << "rhs:\n";
    os << in << "{\n";
    _rhs->print(os, indent + 4);
    os << in << "}\n";
}

reaver::future<> reaver::vapor::analyzer::_v1::binary_expression::_analyze()
{
    return when_all(
            _lhs->analyze(),
            _rhs->analyze()
        ).then([&](auto &&) {
            _overload = resolve_overload(_lhs->get_type(), _rhs->get_type(), _op.type, _scope);
            assert(_overload);

            _set_variable(make_expression_variable(shared_from_this(), _overload->return_type()));
        });
}

