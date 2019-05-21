/**
 * Vapor Compiler Licence
 *
 * Copyright © 2016-2019 Michał "Griwes" Dominiak
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

#include "vapor/analyzer/expressions/module.h"

#include <reaver/future_get.h>
#include <reaver/prelude/monad.h>
#include <reaver/traits.h>

#include "vapor/analyzer/expressions/typeclass.h"
#include "vapor/analyzer/precontext.h"
#include "vapor/analyzer/types/sized_integer.h"
#include "vapor/parser.h"
#include "vapor/parser/module.h"

#include "entity.pb.h"
#include "module.pb.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    std::unique_ptr<module> preanalyze_module(precontext & ctx,
        const parser::module & parse,
        scope * lex_scope)
    {
        std::string cumulative_name;
        module_type * type = nullptr;

        // TODO: integrate this with the same stuff in import_from_ast
        auto name = fmap(parse.name.id_expression_value, [&](auto && elem) {
            auto name_part = elem.value.string;

            cumulative_name = cumulative_name + (cumulative_name.empty() ? "" : ".") + utf8(name_part);
            auto & saved = ctx.modules[cumulative_name];

            if (!saved)
            {
                auto scope = lex_scope->clone_for_class();
                scope->set_name(name_part, codegen::ir::scope_type::module);

                auto old_scope = lex_scope;
                lex_scope = scope.get();

                auto type_uptr = std::make_unique<module_type>(std::move(scope), utf8(name_part));
                type = type_uptr.get();

                saved = make_entity(std::move(type_uptr));
                saved->mark_local();
                assert(old_scope->init(name_part, make_symbol(name_part, saved.get())));
            }

            else
            {
                assert(dynamic_cast<module_type *>(saved->get_type()));
                lex_scope = saved->get_type()->get_scope();
            }

            return name_part;
        });

        auto statements = fmap(parse.statements, [&](const auto & statement) {
            auto scope_ptr = lex_scope;
            auto ret = preanalyze_statement(ctx, statement, scope_ptr);
            assert(lex_scope == scope_ptr);
            return ret;
        });

        return std::make_unique<module>(make_node(parse), std::move(name), type, std::move(statements));
    }

    module::module(ast_node parse,
        std::vector<std::u32string> name,
        module_type * type,
        std::vector<std::unique_ptr<statement>> stmts)
        : expression{ type },
          _parse{ parse },
          _type{ type },
          _name{ std::move(name) },
          _statements{ std::move(stmts) }
    {
    }

    future<> module::_analyze(analysis_context & ctx)
    {
        return when_all(fmap(_statements, [&](auto && stmt) {
            return stmt->analyze(ctx);
        })).then([this, &ctx] {
            if (name() == U"main")
            {
                // set entry(int32) as the entry point
                if (auto entry = _type->get_scope()->try_get(U"entry"))
                {
                    auto type = entry.value()->get_type();
                    return type->get_candidates(lexer::token_type::round_bracket_open)
                        .then([&ctx, expr = entry.value()->get_expression()](auto && overloads) {
                            // maybe this can be relaxed in the future?
                            assert(overloads.size() == 1);
                            assert(overloads[0]->parameters().size() == 1);
                            assert(
                                overloads[0]->parameters()[0]->get_type() == ctx.get_sized_integer_type(32));

                            overloads[0]->mark_as_entry(ctx, expr);
                        });
                }
            }

            return make_ready_future();
        });
    }

    future<expression *> module::_simplify_expr(recursive_context ctx)
    {
        return when_all(fmap(_statements, [&](auto && stmt) {
            return stmt->simplify(ctx).then(
                [&ctx = ctx.proper, &stmt](auto && simplified) { replace_uptr(stmt, simplified, ctx); });
        })).then([this]() -> expression * { return this; });
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
        auto scope = _type->get_scope();
        auto scopes = scope->codegen_ir();

        auto mod = mbind(scope->symbols_in_order(), [&](auto && symbol) {
            return fmap(symbol->codegen_ir(ctx), [&](auto && decl) {
                return fmap(decl,
                    make_overload_set(
                        [&](std::shared_ptr<codegen::ir::variable> symb) {
                            symb->declared = true;
                            symb->scopes = scopes;
                            symb->name = symbol->get_name();
                            return symb;
                        },
                        [&](auto && symb) {
                            symb.scopes = scopes;
                            symb.name = symbol->get_name();
                            return symb;
                        }));
            });
        });

        return mod;
    }

    void module::generate_interface(proto::module & mod) const
    {
        auto scope = _type->get_scope();
        auto own_scope_ir = scope->codegen_ir();

        auto name = fmap(_name, utf8);
        std::copy(name.begin(), name.end(), RepeatedFieldBackInserter(mod.mutable_name()));

        std::unordered_set<expression *> associated_entities;
        std::unordered_set<expression *> exported_entities;
        std::unordered_set<expression *> named_exports;

        auto & mut_symbols = *mod.mutable_symbols();

        for (auto && [_, symbol] : scope->declared_symbols())
        {
            if (!symbol->is_exported())
            {
                continue;
            }

            auto expr = symbol->get_expression();
            exported_entities.insert(expr);
            named_exports.insert(expr);

            auto associated = expr->get_associated_entities();
            associated_entities.insert(associated.begin(), associated.end());
        }

        for (auto && associated : associated_entities)
        {
            if (auto && scope = [&]() -> class scope * {
                    if (auto && type_expr = associated->as<type_expression>())
                    {
                        return type_expr->get_value()->get_scope();
                    }

                    if (auto && tc_expr = associated->as<typeclass_expression>())
                    {
                        return tc_expr->get_typeclass()->get_scope();
                    }

                    return nullptr;
                }())
            {
                auto && scope_ir = scope->codegen_ir();

                if (codegen::ir::same_module(scope_ir, own_scope_ir))
                {
                    exported_entities.insert(associated);
                }
            }

            else
            {
                assert(0);
            }
        }

        for (auto && entity : exported_entities)
        {
            auto & symb = mut_symbols[utf8(entity->get_entity_name())];
            entity->generate_interface(symb);

            auto it = named_exports.find(entity);
            if (it != named_exports.end())
            {
                symb.set_is_name_exported(true);
            }
        }
    }
}
}
