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

#include <reaver/traits.h>
#include <reaver/prelude/monad.h>

#include "vapor/parser.h"
#include "vapor/analyzer/module.h"

reaver::vapor::analyzer::_v1::module::module(const reaver::vapor::parser::module & parse) : _parse{ parse }, _scope{ std::make_unique<reaver::vapor::analyzer::_v1::scope>() }
{
    _statements = fmap(
        _parse.statements,
        [&](const auto & statement){
            auto scope_ptr = _scope.get();
            auto ret = preanalyze_statement(statement, scope_ptr);
            if (scope_ptr != _scope.get())
            {
                _scope.release()->keep_alive();
                _scope.reset(scope_ptr);
            }
            return ret;
        }
    );

    _scope->set_name(name(), codegen::ir::scope_type::module);
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

void reaver::vapor::analyzer::_v1::module::simplify()
{
    bool cont = true;
    while (cont)
    {
        optimization_context ctx{};

        auto all = when_all(fmap(_statements, [&](auto && stmt) {
            return stmt->simplify(ctx);
        }));

        while (!all.try_get())
        {
            std::this_thread::sleep_for(std::chrono::nanoseconds(10000));
        }

        cont = ctx.did_something_happen();
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

namespace
{
    template<typename Map>
    auto as_vector(Map && map)
    {
        std::vector<std::pair<std::u32string, reaver::vapor::analyzer::_v1::symbol *>> ret;
        ret.reserve(map.size());
        std::transform(map.begin(), map.end(), std::back_inserter(ret), [](auto && pair) {
            return std::make_pair(pair.first, pair.second.get());
        });
        return ret;
    }
}

reaver::vapor::codegen::_v1::ir::module reaver::vapor::analyzer::_v1::module::codegen_ir() const
{
    auto ctx = ir_generation_context{};

    codegen::ir::module mod;
    mod.name = fmap(_parse.name.id_expression_value, [&](auto && token) { return token.string; });
    mod.symbols = mbind(as_vector(_scope->declared_symbols()), [&](auto && symbol) {
        auto ir = symbol.second->codegen_ir(ctx);
        fmap(ir.back(), make_overload_set(
            [&](std::shared_ptr<codegen::ir::variable> symb) {
                symb->name = symbol.second->get_name();
                return unit{};
            },
            [&](auto && symb) {
                symb.name = symbol.second->get_name();
                return unit{};
            }
        ));
        return ir;
    });

    while (auto fn = ctx.function_to_generate())
    {
        mod.symbols.push_back(fn->codegen_ir(ctx));
    }

    return mod;
}

