/**
 * Vapor Compiler Licence
 *
 * Copyright © 2016 Michał "Griwes" Dominiak
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

#include "vapor/parser.h"
#include "vapor/analyzer/id_expression.h"

void reaver::vapor::analyzer::_v1::id_expression::print(std::ostream & os, std::size_t indent) const
{
    auto in = std::string(indent, ' ');
    os << in << "id expression `" << utf8(name()) << "` at " << _parse.range << '\n';
    os << in << "referenced variable type: " << get_variable()->get_type()->explain() << '\n';
}

reaver::future<> reaver::vapor::analyzer::_v1::id_expression::_analyze(reaver::vapor::analyzer::_v1::analysis_context & ctx)
{
    return std::accumulate(_parse.id_expression_value.begin() + 1, _parse.id_expression_value.end(),
        _lex_scope->resolve(_parse.id_expression_value.front().string),
        [&](auto fut, auto && ident) {
            return fut.then([&ident](auto && symbol) {
                return symbol->get_variable_future();
            }).then([this, &ident, &ctx](auto && var) {
                return var->get_type()->get_scope()->get_future(ident.string);
            });
        }).then([](auto && symbol) {
            return symbol->get_variable_future();
        }).then([this, &ctx](auto && variable) {
            _referenced = variable;
        });
}

std::unique_ptr<reaver::vapor::analyzer::_v1::expression> reaver::vapor::analyzer::_v1::id_expression::_clone_expr_with_replacement(reaver::vapor::analyzer::_v1::replacements & repl) const
{
    auto referenced = _referenced;

    auto it = repl.variables.find(referenced);
    if (it != repl.variables.end())
    {
        referenced = it->second;
    }

    return make_variable_ref_expression(referenced);
}

reaver::future<reaver::vapor::analyzer::_v1::expression *> reaver::vapor::analyzer::_v1::id_expression::_simplify_expr(reaver::vapor::analyzer::_v1::simplification_context & ctx)
{
    return _referenced->simplify(ctx)
        .then([&](auto && simplified) -> expression * {
            if (simplified && simplified != _referenced)
            {
                _referenced = simplified;
            }
            return this;
        });
}

reaver::vapor::analyzer::_v1::statement_ir reaver::vapor::analyzer::_v1::id_expression::_codegen_ir(reaver::vapor::analyzer::_v1::ir_generation_context & ctx) const
{
    return { codegen::ir::instruction{
        none, none,
        { boost::typeindex::type_id<codegen::ir::pass_value_instruction>() },
        {},
        { get<codegen::ir::value>(_referenced->codegen_ir(ctx)) }
    } };
}

