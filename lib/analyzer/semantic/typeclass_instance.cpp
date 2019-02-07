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

#include "vapor/analyzer/semantic/typeclass_instance.h"
#include "vapor/analyzer/semantic/symbol.h"
#include "vapor/analyzer/statements/function.h"
#include "vapor/analyzer/types/typeclass_instance.h"
#include "vapor/parser/typeclass.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    std::unique_ptr<typeclass_instance> make_typeclass_instance(precontext & ctx,
        const parser::instance_literal & parse,
        scope * lex_scope)
    {
        auto scope = lex_scope->clone_for_class();
        auto scope_ptr = scope.get();

        return std::make_unique<typeclass_instance>(make_node(parse),
            std::move(scope),
            fmap(parse.typeclass_name.id_expression_value, [&](auto && t) { return t.value.string; }),
            fmap(parse.arguments.expressions,
                [&](auto && arg) { return preanalyze_expression(ctx, arg, scope_ptr); }));
    }

    typeclass_instance::typeclass_instance(ast_node parse,
        std::unique_ptr<scope> member_scope,
        std::vector<std::u32string> typeclass_name,
        std::vector<std::unique_ptr<expression>> arguments)
        : _node{ parse },
          _scope{ std::move(member_scope) },
          _typeclass_name{ std::move(typeclass_name) },
          _arguments{ std::move(arguments) }
    {
    }

    std::vector<expression *> typeclass_instance::get_arguments() const
    {
        return fmap(_arguments, [](auto && arg) { return arg.get(); });
    }

    future<> typeclass_instance::simplify_arguments(analysis_context & ctx)
    {
        return when_all(fmap(_arguments, [&](auto && arg) { return simplification_loop(ctx, arg); }))
            .then([](auto &&) {});
    }

    std::vector<function_definition *> typeclass_instance::get_member_function_defs() const
    {
        return fmap(_member_function_definitions, [](auto && def) { return def.get(); });
    }

    void typeclass_instance::set_type(typeclass_instance_type * type)
    {
        for (auto && oset_name : type->overload_set_names())
        {
            _member_overload_sets.push_back(create_overload_set(_scope.get(), oset_name));
        }

        // close here, because if the delayed preanalysis later *adds* new members, then we have a bug... the
        // assertion that checks for closeness of the scope needs to be somehow weakened here, to allow for
        // more sensible error reporting than `assert`
        _scope->close();

        _type = type;
    }

    function_definition_handler typeclass_instance::get_function_definition_handler()
    {
        return [&](precontext & ctx, const parser::function_definition & parse) {
            auto scope = get_scope();
            auto func = preanalyze_function_definition(ctx, parse, scope, _type);
            assert(scope == get_scope());
            _member_function_definitions.push_back(std::move(func));
        };
    }

    void typeclass_instance::import_default_definitions()
    {
        for (auto && oset_name : _type->overload_set_names())
        {
            auto && own_oset = get_overload_set(_scope.get(), oset_name);
            auto && default_oset = get_overload_set(_type->get_scope(), oset_name);

            auto && own_overloads = own_oset->get_overloads();

            for (auto && default_overload : default_oset->get_overloads())
            {
                if (std::find_if(own_overloads.begin(),
                        own_overloads.end(),
                        [&](function * fn) {
                            auto && f1p = fn->parameters();
                            auto && f2p = default_overload->parameters();
                            return std::equal(f1p.begin(),
                                f1p.end(),
                                f2p.begin(),
                                f2p.end(),
                                [](expression * lhs, expression * rhs) {
                                    return lhs->get_type() == rhs->get_type();
                                });
                        })
                    != own_overloads.end())
                {
                    continue;
                }

                auto default_declaration = _type->get_declaration_of(default_overload);
                assert(default_declaration->get_function()->get_body());
                own_oset->add_function(default_declaration);
            }
        }
    }
}
}
