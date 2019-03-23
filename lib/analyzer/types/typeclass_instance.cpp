/**
 * Vapor Compiler Licence
 *
 * Copyright © 2019 Michał "Griwes" Dominiak
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

#include "vapor/analyzer/types/typeclass_instance.h"
#include "vapor/analyzer/expressions/member.h"
#include "vapor/analyzer/semantic/typeclass.h"
#include "vapor/analyzer/semantic/typeclass_instance.h"
#include "vapor/analyzer/statements/block.h"
#include "vapor/analyzer/statements/function.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    typeclass_instance_type::typeclass_instance_type(typeclass * tc, std::vector<expression *> arguments)
        : user_defined_type{ dont_init_expr }, _arguments{ std::move(arguments) }, _ctx{ tc, _arguments }
    {
        // _self_expression = std::make_unique<type_expression>(this, type_kind::typeclass);

        auto repl = _ctx.get_replacements();
        std::unordered_map<function *, block *> function_block_defs;

        for (auto && fn_decl : tc->get_member_function_decls())
        {
            auto fn = fn_decl->get_function();
            auto & name = fn_decl->get_name();

            _function_instance fn_instance;
            fn_instance.instance = make_function(fn->get_explanation(), fn->get_range());
            repl.add_replacement(fn, fn_instance.instance.get());
            fn_instance.return_type_expression = repl.copy_claim(fn->return_type_expression());
            fn_instance.instance->set_return_type(fn_instance.return_type_expression);
            fn_instance.parameter_expressions =
                fmap(fn->parameters(), [&](auto && param) { return repl.copy_claim(param); });
            fn_instance.instance->set_parameters(
                fmap(fn_instance.parameter_expressions, [](auto && expr) { return expr.get(); }));
            fmap(fn->vtable_slot(), [&](auto && id) {
                fn_instance.instance->mark_virtual(id);
                return unit{};
            });

            if (fn->get_body())
            {
                function_block_defs.emplace(fn_instance.instance.get(), fn->get_body());
            }

            fn_instance.overload_set_expression = get_overload_set_special(_oset_scope.get(), name);
            fn_instance.overload_set_expression->get_overload_set()->add_function(fn_instance.instance.get());

            _osets.emplace(name, fn_instance.overload_set_expression->get_overload_set());

            _function_instances.push_back(std::move(fn_instance));
        }

        for (auto && fn_instance : _function_instances)
        {
            auto it = function_block_defs.find(fn_instance.instance.get());
            if (it == function_block_defs.end())
            {
                continue;
            }

            auto body_stmt = repl.copy_claim(it->second);

            auto body_block = dynamic_cast<block *>(body_stmt.get());
            assert(body_block);
            fn_instance.function_body.reset(body_block);
            body_stmt.release();

            fn_instance.instance->set_body(fn_instance.function_body.get());
        }

        _member_expressions.reserve(_osets.size());
        for (auto && [oset_name, oset] : _osets)
        {
            auto member_expression = make_member_expression(this, oset_name, oset->get_type());
            get_scope()->init(oset_name, make_symbol(oset_name, member_expression.get()));
            _member_expressions.push_back(std::move(member_expression));
        }
    }

    std::string typeclass_instance_type::explain() const
    {
        return "a typeclass instance type (TODO: add name tracking to this)";
    }

    void typeclass_instance_type::print(std::ostream & os, print_context ctx) const
    {
        os << styles::def << ctx << styles::type << explain() << '\n';
    }

    future<> typeclass_instance_type::_analyze(analysis_context & ctx)
    {
        return when_all(fmap(_function_instances, [&](auto && instance) {
            return when_all(
                fmap(instance.parameter_expressions, [&](auto && expr) { return expr->analyze(ctx); }))
                .then([&] { return instance.return_type_expression->analyze(ctx); })
                .then([&] {
                    if (instance.function_body)
                    {
                        return instance.function_body->analyze(ctx);
                    }
                    return make_ready_future();
                });
        }));
    }

    std::unique_ptr<google::protobuf::Message> typeclass_instance_type::_user_defined_interface() const
    {
        assert(0);
    }
}
}
