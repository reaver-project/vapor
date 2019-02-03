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
}
}
