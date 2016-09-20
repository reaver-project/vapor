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

#include "vapor/analyzer/closure.h"

void reaver::vapor::analyzer::_v1::closure::print(std::ostream & os, std::size_t indent) const
{
    auto in = std::string(indent, ' ');
    os << in << "closure at " << _parse.range << '\n';
    assert(!_parse.captures);
    assert(!_parse.arguments);
    os << in << "return type: " << _body->return_type()->explain() << '\n';
    os << in << "{\n";
    _body->print(os, indent + 4);
    os << in << "}\n";
}

reaver::future<> reaver::vapor::analyzer::_v1::closure::_analyze()
{
    return _body->analyze().then([&]
    {
        auto function = make_function(
            "closure",
            _body->return_type(),
            {},
            codegen::ir::function{
                U"__closure",
                {},
                _body->codegen_return(),
                _body->codegen_ir()
            },
            _parse.range
        );
        auto type = std::make_shared<closure_type>(shared_from_this(), std::move(function));
        _set_variable(make_expression_variable(shared_from_this(), std::move(type)));
    });
}

reaver::vapor::analyzer::_v1::statement_ir reaver::vapor::analyzer::_v1::closure::_codegen_ir() const
{
    return {
        codegen::ir::instruction{
            none, none,
            { boost::typeindex::type_id<codegen::ir::materialization_instruction>() },
            {},
            { codegen::ir::make_variable(get_variable()->get_type()->codegen_type()) }
        }
    };
}

