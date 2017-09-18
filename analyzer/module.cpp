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

#include <reaver/future_get.h>
#include <reaver/prelude/monad.h>
#include <reaver/traits.h>

#include "vapor/analyzer/module.h"
#include "vapor/parser.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    module::module(const parser::module & parse) : _parse{ parse }, _scope{ std::make_unique<scope>() }
    {
        _statements = fmap(_parse.statements, [&](const auto & statement) {
            auto scope_ptr = _scope.get();
            auto ret = preanalyze_statement(statement, scope_ptr);
            if (scope_ptr != _scope.get())
            {
                _scope.release()->keep_alive();
                _scope.reset(scope_ptr);
            }
            return ret;
        });

        _scope->set_name(name(), codegen::ir::scope_type::module);
        _scope->close();
    }

    void module::analyze(analysis_context & ctx)
    {
        _analysis_futures = fmap(_statements, [&](auto && stmt) { return stmt->analyze(ctx); });

        auto all = when_all(_analysis_futures);
        reaver::get(all);

        // set entry(int32) as the entry point
        if (auto entry = _scope->try_get(U"entry"))
        {
            auto type = entry.get()->get_type();
            auto future = type->get_candidates(lexer::token_type::round_bracket_open);
            auto overloads = reaver::get(future);

            // maybe this can be relaxed in the future?
            assert(overloads.size() == 1);
            assert(overloads[0]->parameters().size() == 1);
            assert(overloads[0]->parameters()[0]->get_type() == ctx.sized_integers.at(32).get());

            overloads[0]->mark_as_entry(ctx, entry.get()->get_expression());
        }

        logger::dlog() << "Analysis of module " << utf8(name()) << " finished.";
    }

    void module::simplify()
    {
        bool cont = true;
        while (cont)
        {
            logger::dlog() << "Simplification run of module " << utf8(name()) << " starting...";

            simplification_context ctx{};

            auto all = when_all(fmap(
                _statements, [&](auto && stmt) { return stmt->simplify({ ctx }).then([&](auto && simplified) { replace_uptr(stmt, simplified, ctx); }); }));
            reaver::get(all);

            cont = ctx.did_something_happen();

            logger::dlog() << "Simplification run of module " << utf8(name()) << " finished.";

            std::stringstream ss;
            print(ss, {});
            logger::dlog() << ss.str();
            logger::default_logger().sync();
        }

        logger::dlog() << "Simplification of module " << utf8(name()) << " finished.";
    }

    void module::print(std::ostream & os, print_context ctx) const
    {
        os << styles::def << ctx << styles::rule_name << "module";
        print_address_range(os, this);
        os << '\n';

        if (_statements.size())
        {
            auto stmts_ctx = ctx.make_branch(true);
            os << styles::def << stmts_ctx << styles::subrule_name << "statements\n";

            std::size_t idx = 0;
            for (auto && stmt : _statements)
            {
                stmt->print(os, stmts_ctx.make_branch(++idx == _statements.size()));
            }
        }
    }

    codegen::_v1::ir::module module::codegen_ir() const
    {
        auto ctx = ir_generation_context{};

        codegen::ir::module mod;
        mod.name = fmap(_parse.name.id_expression_value, [&](auto && ident) { return ident.value.string; });

        mod.symbols = mbind(_scope->symbols_in_order(), [&](auto && symbol) {
            return mbind(symbol->codegen_ir(ctx), [&](auto && decl) {
                return get<0>(fmap(decl,
                    make_overload_set(
                        [&](std::shared_ptr<codegen::ir::variable> symb) {
                            symb->declared = true;
                            symb->name = symbol->get_name();
                            return codegen::ir::module_symbols_t{ symb };
                        },
                        [&](auto && symb) {
                            symb.name = symbol->get_name();
                            return symb;
                        })));
            });
        });

        while (auto fn = ctx.function_to_generate())
        {
            mod.symbols.push_back(fn->codegen_ir(ctx));
        }

        return mod;
    }
}
}
