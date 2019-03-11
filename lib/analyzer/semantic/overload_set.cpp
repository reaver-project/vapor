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

#include "vapor/analyzer/semantic/overload_set.h"
#include "vapor/analyzer/precontext.h"
#include "vapor/analyzer/semantic/function.h"
#include "vapor/analyzer/semantic/symbol.h"
#include "vapor/analyzer/statements/function.h"

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

    std::unique_ptr<overload_set> import_overload_set(precontext & ctx, const proto::overload_set_type & type)
    {
        auto ret = std::make_unique<overload_set>(ctx.module_scope);

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

            imported->function = make_function("overloadable function");
            imported->function->set_return_type(imported->return_type.get());
            imported->function->set_parameters(
                fmap(imported->parameters, [](auto && param) { return param.get(); }));
            imported->function->set_codegen(
                [imported = imported.get()](ir_generation_context & ctx) -> codegen::ir::function {
                    auto params = fmap(imported->parameters, [&](auto && param) {
                        return std::get<std::shared_ptr<codegen::ir::variable>>(
                            param->codegen_ir(ctx).back().result);
                    });

                    auto ret = codegen::ir::function{ U"call",
                        {},
                        std::move(params),
                        codegen::ir::make_variable(
                            imported->return_type->as<type_expression>()->get_value()->codegen_type(ctx)),
                        {} };
                    ret.is_member = true;
                    ret.is_defined = false;

                    return ret;
                });

            if (overload.is_member())
            {
                imported->function->make_member();
            }

            imported->function->set_name(U"call");
            imported->function->set_scopes_generator(
                [type = ret->get_type()](auto && ctx) { return type->codegen_scopes(ctx); });

            ret->add_function(std::move(imported));
        }

        return ret;
    }

    overload_set::overload_set(scope * lex_scope)
        : _type{ std::make_unique<overload_set_type>(lex_scope, this) }
    {
    }

    void overload_set::add_function(function * fn)
    {
        if (std::find_if(_functions.begin(),
                _functions.end(),
                [&](auto && f) { return f->parameters() == fn->parameters(); })
            != _functions.end())
        {
            assert(0);
        }

        _functions.push_back(fn);
    }

    void overload_set::add_function(std::unique_ptr<imported_function> fn)
    {
        add_function(fn->function.get());
        _imported_functions.push_back(std::move(fn));
    }

    void overload_set::add_function(function_declaration * decl)
    {
        add_function(decl->get_function());
        _function_decls.push_back(decl);
    }
}
}
