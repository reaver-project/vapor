/**
 * Vapor Compiler Licence
 *
 * Copyright © 2017-2018 Michał "Griwes" Dominiak
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

#include "vapor/parser/typeclass.h"
#include "vapor/analyzer/expressions/typeclass.h"
#include "vapor/analyzer/statements/function.h"
#include "vapor/analyzer/symbol.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    std::unique_ptr<typeclass_literal> preanalyze_typeclass_literal(precontext & ctx, const parser::typeclass_literal & parse, scope * lex_scope)
    {
        auto scope = lex_scope->clone_for_class();
        auto scope_ptr = scope.get();

        auto ret = std::make_unique<typeclass_literal>(make_node(parse), std::move(scope), fmap(parse.members, [&](auto && member) {
            return std::get<0>(fmap(member,
                make_overload_set([&](const parser::function_declaration & decl)
                                      -> std::unique_ptr<statement> { return preanalyze_function_declaration(ctx, decl, scope_ptr); },
                    [&](const parser::function_definition & def) -> std::unique_ptr<statement> {
                        return preanalyze_function_definition(ctx, def, scope_ptr);
                    })));
        }));

        scope_ptr->close();

        return ret;
    }

    typeclass_literal::typeclass_literal(ast_node parse, std::unique_ptr<scope> lex_scope, std::vector<std::unique_ptr<statement>> declarations)
        : _scope{ std::move(lex_scope) }, _declarations{ std::move(declarations) }
    {
        _set_ast_info(parse);

        auto pair = make_promise<void>();
        _parameters_set = std::move(pair.future);
        _parameters_set_promise = std::move(pair.promise);
    }

    void typeclass_literal::set_template_parameters(std::vector<parameter *> params)
    {
        _params = std::move(params);
        _parameters_set_promise->set();
    }

    void typeclass_literal::print(std::ostream & os, print_context ctx) const
    {
        assert(0);
    }

    statement_ir typeclass_literal::_codegen_ir(ir_generation_context & ctx) const
    {
        assert(0);
    }
}
}
