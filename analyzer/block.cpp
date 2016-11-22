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

#include <numeric>

#include "vapor/analyzer/block.h"
#include "vapor/analyzer/return.h"
#include "vapor/analyzer/symbol.h"
#include "vapor/parser.h"

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
            auto expr = preanalyze_expression(val_expr, _scope.get());
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

    void block::print(std::ostream & os, std::size_t indent) const
    {
        auto in = std::string(indent, ' ');
        os << in << "block at " << _parse.range << '\n';
        os << in << "statements:\n";
        fmap(_statements, [&](auto && stmt) {
            os << in << "{\n";
            stmt->print(os, indent + 4);
            os << in << "}\n";

            return unit{};
        });
        fmap(_value_expr, [&](auto && expr) {
            os << in << "value expression:\n";
            os << in << "{\n";
            expr->print(os, indent + 4);
            os << in << "}\n";

            return unit{};
        });
    }

    future<> block::_analyze(analysis_context & ctx)
    {
        auto fut = foldl(_statements, make_ready_future(), [&ctx](auto && prev, auto && stmt) { return prev.then([&]() { return stmt->analyze(ctx); }); });

        fmap(_value_expr, [&](auto && expr) {
            fut = fut.then([&ctx, expr = expr.get()] { return expr->analyze(ctx); });
            return unit{};
        });

        return fut;
    }

    void block::_ensure_cache() const
    {
        if (_is_clone_cache)
        {
            return;
        }

        std::lock_guard<std::mutex> lock{ _clone_cache_lock };
        if (_clone)
        {
            return;
        }

        auto clone = std::unique_ptr<block>(new block(*this));
        auto repl = replacements{};

        clone->_statements = fmap(_statements, [&](auto && stmt) { return stmt->clone_with_replacement(repl); });
        clone->_value_expr = fmap(_value_expr, [&](auto && expr) { return expr->clone_expr_with_replacement(repl); });
        clone->_is_clone_cache = true;

        _clone = std::move(clone);
    }

    std::unique_ptr<statement> block::_clone_with_replacement(replacements & repl) const
    {
        _ensure_cache();

        if (!_is_clone_cache)
        {
            return _clone.get()->clone_with_replacement(repl);
        }

        auto ret = std::unique_ptr<block>(new block(*this));

        ret->_statements = fmap(_statements, [&](auto && stmt) { return stmt->clone_with_replacement(repl); });
        ret->_value_expr = fmap(_value_expr, [&](auto && expr) { return expr->clone_expr_with_replacement(repl); });

        return ret;
    }

    future<statement *> block::_simplify(simplification_context & ctx)
    {
        _ensure_cache();

        auto fut = foldl(_statements, make_ready_future(true), [&](auto future, auto && statement) {
            return future.then([&](bool do_continue) {
                if (!do_continue)
                {
                    return make_ready_future(false);
                }

                return statement->simplify(ctx).then([&](auto && simplified) {
                    replace_uptr(statement, simplified, ctx);
                    return !statement->always_returns();
                });
            });
        });

        fmap(_value_expr, [&](auto && expr) {
            fut = fut.then([&, expr = expr.get()](bool do_continue) {
                if (!do_continue)
                {
                    return make_ready_future(false);
                }

                return expr->simplify_expr(ctx).then([&](auto && simplified) {
                    replace_uptr(*_value_expr, simplified, ctx);
                    return true;
                });
            });
            return unit{};
        });

        return fut.then([&](bool reached_end) -> statement * {
            if (!reached_end)
            {
                auto always_returning = std::find_if(_statements.begin(), _statements.end(), [](auto && stmt) { return stmt->always_returns(); });

                assert(always_returning != _statements.end());
                _statements.erase(always_returning + 1, _statements.end());
            }

            return this;
        });
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
                auto variable_ir = symbol->get_variable()->codegen_ir(ctx);
                auto variable = get<codegen::ir::value>(variable_ir);
                return codegen::ir::instruction{ {}, {}, { boost::typeindex::type_id<codegen::ir::destruction_instruction>() }, { variable }, variable };
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
            auto to_u32string = [](auto && v) {
                std::stringstream stream;
                stream << v;
                return boost::locale::conv::utf_to_utf<char32_t>(stream.str());
            };

            std::vector<codegen::ir::value> labeled_return_values;

            for (std::size_t i = 0; i < statements.size(); ++i)
            {
                auto & stmt = statements[i];
                if (!stmt.instruction.template is<codegen::ir::return_instruction>())
                {
                    continue;
                }

                std::u32string label;
                if (!stmt.label)
                {
                    label = U"__return_label_" + to_u32string(ctx.label_index++);
                    stmt.label = label;
                }
                else
                {
                    label = stmt.label;
                }

                labeled_return_values.emplace_back(codegen::ir::label{ std::move(label), {} });
                labeled_return_values.emplace_back(stmt.result);
            }

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
                    stmt.operands = { codegen::ir::boolean_value{ true }, codegen::ir::label{ U"__return_phi", {} } };

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

                statements.emplace_back(codegen::ir::instruction{ optional<std::u32string>{ U"__return_phi" },
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
