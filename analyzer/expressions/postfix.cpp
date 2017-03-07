/**
 * Vapor Compiler Licence
 *
 * Copyright © 2016-2017 Michał "Griwes" Dominiak
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

#include <reaver/prelude/fold.h>

#include "vapor/analyzer/expressions/expression_list.h"
#include "vapor/analyzer/expressions/identifier.h"
#include "vapor/analyzer/expressions/postfix.h"
#include "vapor/analyzer/function.h"
#include "vapor/analyzer/helpers.h"
#include "vapor/parser.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    postfix_expression::postfix_expression(const parser::postfix_expression & parse, scope * lex_scope)
        : _parse{ parse }, _scope{ lex_scope }, _modifier{ parse.modifier_type }
    {
        fmap(_parse.base_expression,
            make_overload_set(
                [&](const parser::expression_list & expr_list) {
                    _base_expr = preanalyze_expression_list(expr_list, lex_scope);
                    return unit{};
                },
                [&](const parser::identifier & ident) {
                    _base_expr = preanalyze_identifier(ident, lex_scope);
                    return unit{};
                }));

        if (_parse.arguments.size())
        {
            _arguments = fmap(_parse.arguments, [&](auto && expr) { return preanalyze_expression(expr, lex_scope); });
        }

        fmap(_parse.accessed_member, [&](auto && member) {
            _accessed_member = member.value.string;
            return unit{};
        });
    }

    void postfix_expression::print(std::ostream & os, std::size_t indent) const
    {
        auto in = std::string(indent, ' ');
        os << in << "postfix expression at " << _parse.range << '\n';
        os << in << "type: " << get_variable()->get_type()->explain() << '\n';
        os << in << "base expression:\n";
        os << in << "{\n";
        _base_expr->print(os, indent + 4);
        os << in << "}\n";

        if (_modifier)
        {
            if (_modifier == lexer::token_type::dot)
            {
                os << in << "referenced member: " << utf8(*_accessed_member) << '\n';
                return;
            }

            os << in << "selected call expression:\n";
            os << in << "{\n";
            _call_expression->print(os, indent + 4);
            os << in << "}\n";
            os << in << "modifier type: " << lexer::token_types[+_modifier] << '\n';

            os << in << "arguments:\n";
            fmap(_arguments, [&](auto && arg) {
                os << in << "{\n";
                arg->print(os, indent + 4);
                os << in << "}\n";

                return unit{};
            });
        }
    }

    statement_ir postfix_expression::_codegen_ir(ir_generation_context & ctx) const
    {
        auto base_expr_instructions = _base_expr->codegen_ir(ctx);

        if (!_modifier)
        {
            return base_expr_instructions;
        }

        auto base_variable_value = get<codegen::ir::value>(_base_expr->get_variable()->codegen_ir(ctx));
        auto base_variable = get<std::shared_ptr<codegen::ir::variable>>(base_variable_value);

        if (_modifier == lexer::token_type::dot)
        {
            auto access_instruction = codegen::ir::instruction{ none,
                none,
                { boost::typeindex::type_id<codegen::ir::member_access_instruction>() },
                { base_variable, codegen::ir::label{ _accessed_member.get(), {} } },
                { codegen::ir::make_variable(_referenced_variable.get()->get_type()->codegen_type(ctx)) } };

            base_expr_instructions.push_back(std::move(access_instruction));

            return base_expr_instructions;
        }

        return _call_expression->codegen_ir(ctx);
    }

    variable * postfix_expression::get_variable() const
    {
        if (!_modifier)
        {
            return _base_expr->get_variable();
        }

        if (_referenced_variable)
        {
            return *_referenced_variable;
        }

        return _call_expression->get_variable();
    }
}
}
