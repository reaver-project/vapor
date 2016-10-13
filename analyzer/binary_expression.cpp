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

#include <boost/type_index.hpp>

#include "vapor/parser.h"
#include "vapor/analyzer/binary_expression.h"
#include "vapor/analyzer/function.h"
#include "vapor/analyzer/helpers.h"
#include "vapor/analyzer/symbol.h"

void reaver::vapor::analyzer::_v1::binary_expression::print(std::ostream & os, std::size_t indent) const
{
    auto in = std::string(indent, ' ');
    os << in << "binary expression at " << _parse.range << '\n';
    os << in << "type: " << get_variable()->get_type()->explain() << '\n';
    os << in << "selected overload: " << _overload->explain() << '\n';
    os << in << "lhs:\n";
    os << in << "{\n";
    _lhs->print(os, indent + 4);
    os << in << "}\n";

    os << in << "operator: " << lexer::token_types[+_op.type] << '\n';

    os << in << "rhs:\n";
    os << in << "{\n";
    _rhs->print(os, indent + 4);
    os << in << "}\n";
}

reaver::future<> reaver::vapor::analyzer::_v1::binary_expression::_analyze()
{
    return when_all(
            _lhs->analyze(),
            _rhs->analyze()
        ).then([&](auto &&) {
            return resolve_overload(_lhs->get_type(), _rhs->get_type(), _op.type, _scope);
        }).then([&](auto && overload) {
            _overload = overload;
            return _overload->return_type();
        }).then([&](auto && ret_type) {
            this->_set_variable(make_expression_variable(this, ret_type));
        });
}

reaver::future<reaver::vapor::analyzer::_v1::expression *> reaver::vapor::analyzer::_v1::binary_expression::_simplify_expr(reaver::vapor::analyzer::_v1::optimization_context & ctx)
{
    return when_all(
            _lhs->simplify_expr(ctx),
            _rhs->simplify_expr(ctx)
        ).then([&](auto && simplified) {
            replace_uptr(_lhs, get<0>(simplified), ctx);
            replace_uptr(_rhs, get<1>(simplified), ctx);
            return _overload->simplify(ctx);
        }).then([&]() {
            return _overload->simplify(ctx, { _lhs->get_variable(), _rhs->get_variable() });
        }).then([&](auto && simplified) -> expression * {
            if (simplified)
            {
                ctx.something_heppened();
                return simplified;
            }

            return this;
        });
}

reaver::vapor::analyzer::_v1::statement_ir reaver::vapor::analyzer::_v1::binary_expression::_codegen_ir(reaver::vapor::analyzer::_v1::ir_generation_context & ctx) const
{
    auto lhs_instructions = _lhs->codegen_ir(ctx);
    auto rhs_instructions = _rhs->codegen_ir(ctx);

    auto lhs_variable = lhs_instructions.back().result;
    auto rhs_variable = rhs_instructions.back().result;

    auto bin_expr_instruction = codegen::ir::instruction{
        none, none,
        { boost::typeindex::type_id<codegen::ir::function_call_instruction>() },
        { _overload->call_operand_ir(ctx), lhs_variable, rhs_variable },
        codegen::ir::make_variable((*_overload->return_type().try_get())->codegen_type(ctx))
    };

    // TEMPORARY HACK
    auto cleanup = statement_ir{
        {
            none, none,
            { boost::typeindex::type_id<codegen::ir::temporary_destruction_instruction>() },
            { rhs_variable },
            rhs_variable
        },
        {
            none, none,
            { boost::typeindex::type_id<codegen::ir::temporary_destruction_instruction>() },
            { lhs_variable },
            lhs_variable
        }
    };

    // THIS IS ALSO A PART OF THE TEMPORARY HACK
    auto pass_value = codegen::ir::instruction{
        none, none,
        { boost::typeindex::type_id<codegen::ir::pass_value_instruction>() },
        { bin_expr_instruction.result },
        bin_expr_instruction.result
    };

    ctx.add_function_to_generate(_overload);

    // TODO: oops, there should be a different order of evaluation for right-associative and left-associative
    // (...probably)
    statement_ir ret;
    ret.reserve(lhs_instructions.size() + rhs_instructions.size() + 1 + 3);
    std::move(lhs_instructions.begin(), lhs_instructions.end(), std::back_inserter(ret));
    std::move(rhs_instructions.begin(), rhs_instructions.end(), std::back_inserter(ret));
    ret.push_back(std::move(bin_expr_instruction));
    std::move(cleanup.begin(), cleanup.end(), std::back_inserter(ret));
    ret.push_back(std::move(pass_value));

    return ret;
}

