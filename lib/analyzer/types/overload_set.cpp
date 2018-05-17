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

#include <numeric>

#include "vapor/analyzer/expressions/entity.h"
#include "vapor/analyzer/expressions/expression.h"
#include "vapor/analyzer/expressions/type.h"
#include "vapor/analyzer/function.h"
#include "vapor/analyzer/helpers.h"
#include "vapor/analyzer/precontext.h"
#include "vapor/analyzer/symbol.h"
#include "vapor/analyzer/types/overload_set.h"
#include "vapor/analyzer/types/unresolved.h"
#include "vapor/codegen/ir/type.h"

#include "types/overload_set.pb.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    struct imported_function
    {
        std::unique_ptr<expression> return_type;
        std::vector<std::unique_ptr<expression>> parameters;
        std::unique_ptr<class function> function;
    };

    std::unique_ptr<overload_set_type> import_overload_set_type(precontext & ctx, const proto::overload_set_type & type)
    {
        auto ret = std::make_unique<overload_set_type>(ctx.module_scope);

        assert(type.functions_size() != 0);

        for (auto && overload : type.functions())
        {
            auto imported = std::make_unique<imported_function>();
            imported->return_type = get_imported_type_ref_expr(ctx, overload.return_type());

            for (auto && param : overload.parameters())
            {
                auto type = get_imported_type_ref(ctx, param.type());
                imported->parameters.push_back(make_entity(std::move(type)));
            }

            imported->function = make_function("overloadable function",
                imported->return_type.get(),
                fmap(imported->parameters, [](auto && param) { return param.get(); }),
                [imported = imported.get()](ir_generation_context & ctx)->codegen::ir::function {
                    auto params = fmap(imported->parameters,
                        [&](auto && param) { return std::get<std::shared_ptr<codegen::ir::variable>>(param->codegen_ir(ctx).back().result); });
                    // this is slightly messy, but cleaning it up before I have typeclasses would be wasteful
                    // since I can fix it in an even better way when I have those
                    params.erase(params.begin());

                    auto ret = codegen::ir::function{ U"call",
                        {},
                        std::move(params),
                        codegen::ir::make_variable(imported->return_type->as<type_expression>()->get_value()->codegen_type(ctx)),
                        {} };
                    ret.is_member = true;
                    ret.is_defined = false;

                    return ret;
                });

            assert(overload.is_member());

            imported->function->set_name(U"call");
            imported->function->make_member();
            imported->function->set_scopes_generator([type = ret.get()](auto && ctx) { return type->codegen_scopes(ctx); });

            ret->add_function(std::move(imported));
        }

        return ret;
    }

    void overload_set_type::add_function(function * fn)
    {
        if (std::find_if(_functions.begin(), _functions.end(), [&](auto && f) { return f->parameters() == fn->parameters(); }) != _functions.end())
        {
            assert(0);
        }

        _functions.push_back(fn);
    }

    void overload_set_type::add_function(std::unique_ptr<imported_function> fn)
    {
        add_function(fn->function.get());
        _imported_functions.push_back(std::move(fn));
    }

    future<std::vector<function *>> overload_set_type::get_candidates(lexer::token_type bracket) const
    {
        if (bracket == lexer::token_type::round_bracket_open)
        {
            assert(_functions.size());
            return make_ready_future([&] { return _functions; }());
        }

        assert(0);
        return make_ready_future(std::vector<function *>{});
    }

    void overload_set_type::_codegen_type(ir_generation_context & ctx) const
    {
        auto actual_type = *_codegen_t;
        auto members = fmap(_functions, [&](auto && fn) {
            ctx.add_generated_function(fn);
            return codegen::ir::member{ fn->codegen_ir(ctx) };
        });
        auto type = codegen::ir::variable_type{ _codegen_name(ctx), get_scope()->codegen_ir(), 0, std::move(members) };

        auto scopes = get_scope()->codegen_ir();
        scopes.emplace_back(type.name, codegen::ir::scope_type::type);

        fmap(type.members, [&](auto && member) {
            fmap(member,
                make_overload_set(
                    [&](codegen::ir::function & fn) {
                        fn.scopes = scopes;
                        fn.parent_type = actual_type;
                        fn.is_exported = _is_exported;

                        return unit{};
                    },
                    [&](auto &&) {
                        assert(0);
                        return unit{};
                    }));
            return unit{};
        });

        *actual_type = std::move(type);
    }

    void overload_set_type::print(std::ostream & os, print_context ctx) const
    {
        os << styles::def << ctx << styles::type << "overload set type";
        os << styles::def << " @ " << styles::address << this << styles::def << ":\n";

        std::size_t idx = 0;
        for (auto && function : _functions)
        {
            function->print(os, ctx.make_branch(++idx == _functions.size()));
        }
    }

    std::unique_ptr<google::protobuf::Message> overload_set_type::_user_defined_interface() const
    {
        auto t = std::make_unique<proto::overload_set_type>();

        for (auto && func : _functions)
        {
            auto fn = t->add_functions();
            fn->set_allocated_return_type(
                func->get_return_type().try_get().value()->as<type_expression>()->get_value()->generate_interface_reference().release());

            for (auto && param : func->parameters())
            {
                auto & par = *fn->add_parameters();
                par.set_allocated_type(param->get_type()->generate_interface_reference().release());
            }

            fn->set_is_member(func->is_member());
        }

        return t;
    }
}
}
