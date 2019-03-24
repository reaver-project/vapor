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

#include "vapor/analyzer/semantic/typeclass.h"

#include "vapor/analyzer/precontext.h"
#include "vapor/analyzer/semantic/parameter_list.h"
#include "vapor/analyzer/statements/function.h"
#include "vapor/analyzer/types/typeclass_instance.h"
#include "vapor/parser/expr.h"
#include "vapor/parser/typeclass.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    std::unique_ptr<typeclass> make_typeclass(precontext & ctx,
        const parser::typeclass_literal & parse,
        scope * lex_scope)
    {
        auto scope = lex_scope->clone_for_class();
        auto scope_ptr = scope.get();

        auto params = preanalyze_parameter_list(ctx, parse.parameters, scope_ptr);

        std::vector<std::unique_ptr<function_declaration>> fn_decls;

        fmap(parse.members, [&](auto && member) {
            fmap(member,
                make_overload_set(
                    [&](const parser::function_declaration & decl) {
                        fn_decls.push_back(preanalyze_function_declaration(ctx, decl, scope_ptr));
                        return unit{};
                    },
                    [&](const parser::function_definition & def) {
                        fn_decls.push_back(preanalyze_function_definition(ctx, def, scope_ptr));
                        return unit{};
                    }));

            return unit{};
        });

        scope_ptr->close();

        std::unordered_map<std::u32string, std::size_t> overload_set_function_counts;
        for (auto && fn_decl : fn_decls)
        {
            fn_decl->get_function()->mark_virtual(overload_set_function_counts[fn_decl->get_name()]++);
        }

        return std::make_unique<typeclass>(
            make_node(parse), std::move(scope), std::move(params), std::move(fn_decls));
    }

    typeclass::typeclass(ast_node parse,
        std::unique_ptr<scope> member_scope,
        std::vector<std::unique_ptr<parameter>> parameters,
        std::vector<std::unique_ptr<function_declaration>> member_function_decls)
        : _parse{ parse },
          _scope{ std::move(member_scope) },
          _parameters{ std::move(parameters) },
          _member_function_declarations{ std::move(member_function_decls) }
    {
    }

    typeclass::~typeclass() = default;

    void typeclass::print(std::ostream & os, print_context ctx, bool print_members) const
    {
        os << styles::def << ctx << styles::type << "typeclass";
        print_address_range(os, this);
        os << '\n';

        if (print_members)
        {
            auto osets_ctx = ctx.make_branch(true);
            os << styles::def << osets_ctx << styles::subrule_name << "overload sets:\n";

            std::vector<overload_set *> osets;
            for (auto && symbol : _scope->symbols_in_order())
            {
                if (auto oset = symbol->get_expression()->as<overload_set_expression>())
                {
                    osets.push_back(oset->get_overload_set());
                }
            }

            std::size_t idx = 0;
            for (auto && oset : osets)
            {
                oset->print(os, osets_ctx.make_branch(++idx == osets.size()), true);
            }
        }
    }

    std::vector<parameter *> typeclass::get_parameters() const
    {
        return fmap(_parameters, [](auto && param) { return param.get(); });
    }

    std::vector<expression *> typeclass::get_parameter_expressions() const
    {
        return fmap(_parameters, [](auto && param) -> expression * { return param.get(); });
    }

    future<typeclass_instance_type *> typeclass::type_for(analysis_context & ctx,
        const std::vector<expression *> & arguments)
    {
        auto & type_info = _instance_types[arguments];
        if (!type_info.instance)
        {
            type_info.instance = std::make_unique<typeclass_instance_type>(this, arguments);
            type_info.analysis_future =
                type_info.instance->_analyze(ctx).then([i = type_info.instance.get()] { return i; });
        }

        return type_info.analysis_future.value();
    }
}
}

