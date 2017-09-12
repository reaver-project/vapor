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

#include <numeric>

#include <reaver/prelude/fold.h>

#include "vapor/analyzer/expressions/expression_list.h"
#include "vapor/analyzer/statements/block.h"
#include "vapor/analyzer/statements/return.h"
#include "vapor/analyzer/symbol.h"
#include "vapor/parser.h"
#include "vapor/utf.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    block::block(const parser::block & parse, scope * lex_scope, bool is_top_level)
        : _parse{ parse }, _scope{ lex_scope->clone_local() }, _original_scope{ _scope.get() }, _is_top_level{ is_top_level }
    {
        _statements = fmap(_parse.block_value, [&](auto && row) {
            return get<0>(fmap(row,
                make_overload_set([&](const parser::block & block) -> std::unique_ptr<statement> { return preanalyze_block(block, _scope.get(), false); },
                    [&](const parser::statement & statement) {
                        auto scope_ptr = _scope.get();
                        auto ret = preanalyze_statement(statement, scope_ptr);
                        if (scope_ptr != _scope.get())
                        {
                            _scope.release()->keep_alive();
                            _scope.reset(scope_ptr);
                        }
                        return ret;
                    })));
        });

        _scope->close();

        _value_expr = fmap(_parse.value_expression, [&](auto && val_expr) {
            auto expr = preanalyze_expression_list(val_expr, _scope.get());
            return expr;
        });
    }

    type * block::return_type() const
    {
        auto return_types = fmap(get_returns(), [](auto && stmt) { return stmt->get_returned_type(); });

        auto val = value_type();
        if (val)
        {
            return_types.push_back(val);
        }

        std::sort(return_types.begin(), return_types.end());
        return_types.erase(std::unique(return_types.begin(), return_types.end()), return_types.end());
        assert(return_types.size() == 1);
        return return_types.front();
    }

    void block::print(std::ostream & os, print_context ctx) const
    {
        os << styles::def << ctx << styles::rule_name << "block";
        print_address_range(os, this);
        os << '\n';

        auto stmts_ctx = ctx.make_branch(!_value_expr);
        os << styles::def << stmts_ctx << styles::subrule_name << "statements:\n";

        std::size_t idx = 0;
        for (auto && stmt : _statements)
        {
            stmt->print(os, stmts_ctx.make_branch(++idx == _statements.size()));
        }

        if (_value_expr)
        {
            auto value_ctx = ctx.make_branch(true);
            os << styles::def << value_ctx << styles::subrule_name << "value expression:\n";

            _value_expr.get()->print(os, value_ctx.make_branch(true));
        }
    }

    std::vector<codegen::_v1::ir::instruction> block::_codegen_ir(ir_generation_context & ctx) const
    {
        auto statements = mbind(_statements, [&](auto && stmt) { return stmt->codegen_ir(ctx); });
        fmap(_value_expr, [&](auto && expr) {
            auto instructions = expr->codegen_ir(ctx);
            instructions.emplace_back(
                codegen::ir::instruction{ none, none, { boost::typeindex::type_id<codegen::ir::return_instruction>() }, {}, instructions.back().result });
            std::move(instructions.begin(), instructions.end(), std::back_inserter(statements));
            return unit{};
        });

        statement_ir scope_cleanup;
        for (auto scope = _scope.get(); scope != _original_scope->parent(); scope = scope->parent())
        {
            std::transform(scope->symbols_in_order().rbegin(), scope->symbols_in_order().rend(), std::back_inserter(scope_cleanup), [&ctx](auto && symbol) {
                auto ir = symbol->get_expression()->codegen_ir(ctx).back().result;
                return codegen::ir::instruction{ {}, {}, { boost::typeindex::type_id<codegen::ir::destruction_instruction>() }, { ir }, ir };
            });
        }

        for (std::size_t i = 0; i < statements.size(); ++i)
        {
            auto & stmt = statements[i];
            if (!stmt.instruction.template is<codegen::ir::return_instruction>())
            {
                continue;
            }

            statements.insert(statements.begin() + i, scope_cleanup.begin(), scope_cleanup.end());
            i += scope_cleanup.size();
        }

        if (_is_top_level)
        {
            std::vector<codegen::ir::value> labeled_return_values;
            std::u32string_view current_label = U"entry";

            for (std::size_t i = 0; i < statements.size(); ++i)
            {
                auto & stmt = statements[i];

                if (stmt.label)
                {
                    current_label = *stmt.label;
                }

                if (!stmt.instruction.template is<codegen::ir::return_instruction>())
                {
                    continue;
                }

                std::u32string label;
                if (!stmt.label)
                {
                    label = current_label;
                }
                else
                {
                    label = stmt.label;
                }

                labeled_return_values.emplace_back(codegen::ir::label{ label, {} });
                labeled_return_values.emplace_back(stmt.result);
            }

            // in this case we don't generate jumps
            // if we generate the unnecessary label, it'll turn into an LLVM block
            // and LLVM blocks need to have a branch before them
            // hence, remove the label if it's useless
            if (labeled_return_values.size() > 2)
            {
                std::size_t return_value_index = 0;

                for (std::size_t i = 0; i < statements.size(); ++i)
                {
                    auto & stmt = statements[i];

                    if (!stmt.instruction.template is<codegen::ir::return_instruction>())
                    {
                        continue;
                    }

                    stmt.instruction = boost::typeindex::type_id<codegen::ir::jump_instruction>();
                    stmt.operands = { codegen::ir::label{ U"return_phi", {} } };

                    // create a variable for the constant return value
                    if (stmt.result.index() != 0)
                    {
                        auto old_result = stmt.result;
                        auto var = make_variable(get_type(old_result));
                        stmt.result = var;
                        statements.insert(statements.begin() + i++,
                            { {}, {}, { boost::typeindex::type_id<codegen::ir::materialization_instruction>() }, { old_result }, var });
                        labeled_return_values[2 * return_value_index + 1] = var;
                    }

                    ++return_value_index;
                }

                statements.emplace_back(codegen::ir::instruction{ optional<std::u32string>{ U"return_phi" },
                    none,
                    { boost::typeindex::type_id<codegen::ir::phi_instruction>() },
                    std::move(labeled_return_values),
                    codegen::ir::make_variable(return_type()->codegen_type(ctx)) });

                statements.emplace_back(
                    codegen::ir::instruction{ none, none, { boost::typeindex::type_id<codegen::ir::return_instruction>() }, {}, statements.back().result });
            }
        }

        return statements;
    }

    codegen::_v1::ir::value block::codegen_return(ir_generation_context & ctx) const
    {
        assert(_is_top_level);
        return codegen_ir(ctx).back().result;
    }
}
}
