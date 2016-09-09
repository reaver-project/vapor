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

#include "vapor/analyzer/module.h"

reaver::vapor::analyzer::_v1::module::module(const reaver::vapor::parser::module & parse) : _parse{ parse }, _scope{ std::make_shared<reaver::vapor::analyzer::_v1::scope>() }
{
    _statements = fmap(
        _parse.statements,
        [&](const auto & statement){
            return preanalyze_statement(statement, _scope);
        }
    );

    _scope->close();
}

void reaver::vapor::analyzer::_v1::module::analyze()
{
    _analysis_futures = fmap(_statements, [&](auto && stmt) {
        return stmt->analyze();
    });

    auto all = when_all(_analysis_futures);

    while (!all.try_get())
    {
        std::this_thread::sleep_for(std::chrono::nanoseconds(10000));
    }
}

void reaver::vapor::analyzer::_v1::module::print(std::ostream & os, std::size_t indent) const
{
    auto in = std::string(indent, ' ');
    os << in << "module `" << utf8(name()) << "` at " << _parse.range << '\n';
    fmap(_statements, [&](auto && stmt) {
        os << in << "{\n";
        stmt->print(os, indent + 4);
        os << in << "}\n";

        return unit{};
    });
}
