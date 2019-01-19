/**
 * Vapor Compiler Licence
 *
 * Copyright © 2016-2019 Michał "Griwes" Dominiak
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

#include "vapor/analyzer/statements/if.h"
#include "vapor/analyzer/helpers.h"
#include "vapor/analyzer/semantic/symbol.h"
#include "vapor/parser/expr.h"
#include "vapor/parser/if_statement.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    std::unique_ptr<if_statement> preanalyze_if_statement(precontext & ctx, const parser::if_statement & parse, scope * lex_scope)
    {
        return std::make_unique<if_statement>(make_node(parse),
            preanalyze_expression(ctx, parse.condition, lex_scope),
            preanalyze_block(ctx, parse.then_block, lex_scope, false),
            fmap(parse.else_block, [&](auto && parse) -> std::unique_ptr<statement> { return preanalyze_block(ctx, parse, lex_scope, false); }));
    }

    if_statement::if_statement(ast_node parse,
        std::unique_ptr<expression> condition,
        std::unique_ptr<statement> then,
        std::optional<std::unique_ptr<statement>> else_)
        : _condition{ std::move(condition) }, _then_block{ std::move(then) }, _else_block{ std::move(else_) }
    {
        _set_ast_info(parse);
    }

    void if_statement::print(std::ostream & os, print_context ctx) const
    {
        os << styles::def << ctx << styles::rule_name << "if-statement";
        print_address_range(os, this);
        os << '\n';

        auto condition_ctx = ctx.make_branch(false);
        os << styles::def << condition_ctx << styles::subrule_name << "condition:\n";
        _condition->print(os, condition_ctx.make_branch(true));

        auto then_ctx = ctx.make_branch(!_else_block);
        os << styles::def << then_ctx << styles::subrule_name << "then block:\n";
        _then_block->print(os, then_ctx.make_branch(true));

        if (_else_block)
        {
            auto else_ctx = ctx.make_branch(true);
            os << styles::def << else_ctx << styles::subrule_name << "else block:\n";
            _else_block.value()->print(os, ctx.make_branch(true));
        }
    }

    statement_ir if_statement::_codegen_ir(ir_generation_context & ctx) const
    {
        // instructions returning those labels are silly
        // really need to do this better at some point
        // (future refactor perhaps)
        // (perhaps once I have tuples, so they can return unit type values)

        auto condition_instructions = _condition->codegen_ir(ctx);
        auto condition_variable = condition_instructions.back().result;

        auto then_label = U"then_" + utf32(std::to_string(ctx.label_index++));
        auto else_label = U"else_" + utf32(std::to_string(ctx.label_index++));
        auto after_else_label = U"after_else_" + utf32(std::to_string(ctx.label_index++));

        auto then_instructions = [&]() {
            auto ir = _then_block->codegen_ir(ctx);
            ir.front().label = then_label;
            return ir;
        }();

        auto else_instructions = [&]() {
            statement_ir ir;
            if (_else_block)
            {
                ir = _else_block.value()->codegen_ir(ctx);
            }
            else
            {
                ir = statement_ir{ { {}, {}, { boost::typeindex::type_id<codegen::ir::noop_instruction>() }, {}, codegen::ir::label{ else_label, {} } } };
            }

            ir.front().label = else_label;
            return ir;
        }();

        auto jump = codegen::ir::instruction{ {},
            {},
            { boost::typeindex::type_id<codegen::ir::conditional_jump_instruction>() },
            { condition_variable, codegen::ir::label{ then_label, {} }, codegen::ir::label{ else_label, {} } },
            condition_variable };

        auto jump_after_else = codegen::ir::instruction{
            {}, {}, { boost::typeindex::type_id<codegen::ir::jump_instruction>() }, { codegen::ir::label{ after_else_label, {} } }, condition_variable
        };

        auto after_else = codegen::ir::instruction{
            after_else_label, {}, { boost::typeindex::type_id<codegen::ir::noop_instruction>() }, {}, codegen::ir::label{ after_else_label, {} }
        };

        statement_ir ret;
        ret.reserve(condition_instructions.size() + 4 + then_instructions.size() + else_instructions.size());
        std::move(condition_instructions.begin(), condition_instructions.end(), std::back_inserter(ret));
        ret.push_back(std::move(jump));
        std::move(then_instructions.begin(), then_instructions.end(), std::back_inserter(ret));
        ret.push_back(jump_after_else);
        std::move(else_instructions.begin(), else_instructions.end(), std::back_inserter(ret));
        ret.push_back(std::move(jump_after_else));
        ret.push_back(std::move(after_else));
        return ret;
    }
}
}
