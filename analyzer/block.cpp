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
#include "vapor/analyzer/block.h"
#include "vapor/analyzer/return.h"

reaver::vapor::analyzer::_v1::block::block(const reaver::vapor::parser::block & parse, std::shared_ptr<reaver::vapor::analyzer::_v1::scope> lex_scope) : _parse{ parse }, _scope{ lex_scope->clone_local() }
{
    _statements = fmap(_parse.block_value, [&](auto && row) {
        return get<0>(fmap(row, make_overload_set(
            [&](const parser::block & block) -> std::shared_ptr<statement>
            {
                return preanalyze_block(block, _scope);
            },
            [&](const parser::statement & statement)
            {
                return preanalyze_statement(statement, _scope);
            }
        )));
    });

    _scope->close();

    _value_expr = fmap(_parse.value_expression, [&](auto && val_expr) {
        auto expr = preanalyze_expression(val_expr, _scope);
        return expr;
    });
}

std::shared_ptr<reaver::vapor::analyzer::_v1::type> reaver::vapor::analyzer::_v1::block::return_type() const
{
    auto return_types = fmap(get_returns(), [](auto && stmt){ return stmt->get_returned_type(); });

    auto val = value_type();
    if (val)
    {
        return_types.push_back(val);
    }

    std::sort(return_types.begin(), return_types.end());
    return_types.erase(std::unique(return_types.begin(), return_types.end()), return_types.end());
    assert(return_types.size() == 1);
    return return_types.front();
}

void reaver::vapor::analyzer::_v1::block::print(std::ostream & os, std::size_t indent) const
{
    auto in = std::string(indent, ' ');
    os << in << "block at " << _parse.range << '\n';
    os << in << "statements:\n";
    fmap(_statements, [&](auto && stmt) {
        os << in << "{\n";
        stmt->print(os, indent + 4);
        os << in << "}\n";

        return unit{};
    });
    fmap(_value_expr, [&](auto && expr) {
        os << in << "value expression:\n";
        os << in << "{\n";
        expr->print(os, indent + 4);
        os << in << "}\n";

        return unit{};
    });
}

reaver::future<> reaver::vapor::analyzer::_v1::block::_analyze()
{
    auto fut = when_all(
        fmap(_statements, [&](auto && stmt) { return stmt->analyze(); })
    );

    fmap(_value_expr, [&](auto && expr) {
        fut = fut.then([expr]{ return expr->analyze(); });
        return unit{};
    });

    return fut;
}

