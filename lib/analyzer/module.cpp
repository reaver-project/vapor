/**
 * Vapor Compiler Licence
 *
 * Copyright © 2016-2018 Michał "Griwes" Dominiak
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
#include "vapor/parser/module.h"

#include "module.pb.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    std::unique_ptr<module> preanalyze_module(precontext & ctx, const parser::module & parse, scope * lex_scope)
    {
        auto scope = lex_scope->clone_for_class();
        auto name = fmap(parse.name.id_expression_value, [](auto && elem) { return elem.value.string; });

        auto statements = fmap(parse.statements, [&](const auto & statement) {
            auto scope_ptr = scope.get();
            auto ret = preanalyze_statement(ctx, statement, scope_ptr);
            if (scope_ptr != scope.get())
            {
                scope.release()->keep_alive();
                scope.reset(scope_ptr);
            }
            return ret;
        });

        scope->set_name(boost::join(name, "."), codegen::ir::scope_type::module);
        scope->close();

        return std::make_unique<module>(make_node(parse), std::move(name), std::move(scope), std::move(statements));
    }

    module::module(ast_node parse, std::vector<std::u32string> name, std::unique_ptr<scope> lex_scope, std::vector<std::unique_ptr<statement>> stmts)
        : _parse{ parse }, _name{ std::move(name) }, _scope{ std::move(lex_scope) }, _statements{ std::move(stmts) }
    {
    }

    future<> module::_analyze(analysis_context & ctx)
    {
        return when_all(fmap(_statements, [&](auto && stmt) { return stmt->analyze(ctx); })).then([this, &ctx] {
            if (name() == U"main")
            {
                // set entry(int32) as the entry point
                if (auto entry = _scope->try_get(U"entry"))
                {
                    auto type = entry.value()->get_type();
                    return type->get_candidates(lexer::token_type::round_bracket_open).then([&ctx, expr = entry.value()->get_expression()](auto && overloads) {
                        // maybe this can be relaxed in the future?
                        assert(overloads.size() == 1);
                        assert(overloads[0]->parameters().size() == 1);
                        assert(overloads[0]->parameters()[0]->get_type() == ctx.sized_integers.at(32).get());

                        overloads[0]->mark_as_entry(ctx, expr);
                    });
                }
            }

            return make_ready_future();
        });
    }

    future<statement *> module::_simplify(recursive_context ctx)
    {
        return when_all(fmap(_statements,
                            [&](auto && stmt) {
                                return stmt->simplify(ctx).then([&ctx = ctx.proper, &stmt](auto && simplified) { replace_uptr(stmt, simplified, ctx); });
                            }))
            .then([this]() -> statement * { return this; });
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

    declaration_ir module::declaration_codegen_ir(ir_generation_context & ctx) const
    {
        codegen::ir::module mod;
        mod.name = _name;
        mod.symbols = mbind(_scope->symbols_in_order(), [&](auto && symbol) {
            return mbind(symbol->codegen_ir(ctx), [&](auto && decl) {
                return get<0>(fmap(decl,
                    make_overload_set(
                        [&](std::shared_ptr<codegen::ir::variable> symb) {
                            symb->declared = true;
                            symb->name = symbol->get_name();
                            return codegen::ir::module_symbols_t{ symb };
                        },
                        [&](codegen::ir::module &) -> codegen::ir::module_symbols_t { assert(0); },
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

        return { { mod } };
    }

    void module::generate_interface(proto::module & mod) const
    {
        auto name = fmap(_name, utf8);
        std::copy(name.begin(), name.end(), RepeatedFieldBackInserter(mod.mutable_name()));

        auto & mut_symbols = *mod.mutable_symbols();

        for (auto && symbol : _scope->declared_symbols())
        {
            if (!symbol.second->is_exported())
            {
                continue;
            }

            mut_symbols[utf8(symbol.first)];
        }
    }
}
}
