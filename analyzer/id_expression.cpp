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

reaver::future<> reaver::vapor::analyzer::_v1::id_expression::_analyze()
{
    return std::accumulate(_parse.id_expression_value.begin() + 1, _parse.id_expression_value.end(), _lex_scope->resolve(_parse.id_expression_value.front().string),
        [&](auto fut, auto && ident) {
            return fut.then([&ident](auto && symbol) {
                return symbol->get_variable_future();
            }).then([&ident](auto && var) {
                return var->get_type()->get_scope()->get_future(ident.string);
            });
        }).then([](auto && symbol) {
            return symbol->get_variable_future();
        }).then([this](auto && variable) {
            this->_set_variable(variable);
        });
}

reaver::future<std::shared_ptr<reaver::vapor::analyzer::_v1::expression>> reaver::vapor::analyzer::_v1::id_expression::_simplify_expr(reaver::vapor::analyzer::_v1::optimization_context & ctx)
{
    return get_variable()->simplify(ctx)
        .then([&](auto && simplified) {
            this->_set_variable(std::move(simplified));
            return _shared_from_this();
        });
}

reaver::vapor::analyzer::_v1::statement_ir reaver::vapor::analyzer::_v1::id_expression::_codegen_ir(reaver::vapor::analyzer::_v1::ir_generation_context & ctx) const
{
    auto result = codegen::ir::make_variable(get_variable()->get_type()->codegen_type(ctx), name());
    result->declared = true;
    return { codegen::ir::instruction{
        none, none,
        { boost::typeindex::type_id<codegen::ir::pass_value_instruction>() },
        {},
        { std::move(result) }
    } };
}

