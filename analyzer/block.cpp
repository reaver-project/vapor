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
#include "vapor/analyzer/block.h"
#include "vapor/analyzer/return.h"
#include "vapor/analyzer/symbol.h"

reaver::vapor::analyzer::_v1::block::block(const reaver::vapor::parser::block & parse, reaver::vapor::analyzer::_v1::scope * lex_scope, bool is_top_level) : _parse{ parse }, _scope{ lex_scope->clone_local() }, _is_top_level{ is_top_level }
{
    _statements = fmap(_parse.block_value, [&](auto && row) {
        return get<0>(fmap(row, make_overload_set(
            [&](const parser::block & block) -> std::unique_ptr<statement>
            {
                return preanalyze_block(block, _scope.get(), false);
            },
            [&](const parser::statement & statement)
            {
                auto scope_ptr = _scope.get();
                auto ret = preanalyze_statement(statement, scope_ptr);
                if (scope_ptr != _scope.get())
                {
                    _scope.release()->keep_alive();
                    _scope.reset(scope_ptr);
                }
                return ret;
            }
        )));
    });

    _scope->close();

    _value_expr = fmap(_parse.value_expression, [&](auto && val_expr) {
        auto expr = preanalyze_expression(val_expr, _scope.get());
        return expr;
    });
}

reaver::vapor::analyzer::_v1::type * reaver::vapor::analyzer::_v1::block::return_type() const
{
    auto return_types = fmap(get_returns(), [](auto && stmt){ return stmt->get_returned_type(); });

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

void reaver::vapor::analyzer::_v1::block::print(std::ostream & os, std::size_t indent) const
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

reaver::future<> reaver::vapor::analyzer::_v1::block::_analyze()
{
    auto fut = when_all(
        fmap(_statements, [&](auto && stmt) { return stmt->analyze(); })
    );

    fmap(_value_expr, [&](auto && expr) {
        fut = fut.then([expr = expr.get()]{ return expr->analyze(); });
        return unit{};
    });

    return fut;
}

reaver::future<reaver::vapor::analyzer::_v1::statement *> reaver::vapor::analyzer::_v1::block::_simplify(reaver::vapor::analyzer::_v1::optimization_context & ctx)
{
    auto fut = when_all(
        fmap(_statements, [&](auto && stmt) { return stmt->simplify(ctx); })
    ).then([&](auto && simplified) {
        replace_uptrs(_statements, simplified, ctx);
    });

    fmap(_value_expr, [&](auto && expr) {
        fut = fut.then([&, expr = expr.get()]{
            return expr->simplify_expr(ctx);
        }).then([&](auto && simplified) {
            replace_uptr(*_value_expr, simplified, ctx);
        });
        return unit{};
    });

    return fut.then([&]() -> statement * {
        return this;
    });
}

std::vector<reaver::vapor::codegen::_v1::ir::instruction> reaver::vapor::analyzer::_v1::block::_codegen_ir(reaver::vapor::analyzer::_v1::ir_generation_context & ctx) const
{
    auto statements = mbind(_statements, [&](auto && stmt) {
        return stmt->codegen_ir(ctx);
    });
    fmap(_value_expr, [&](auto && expr) {
        auto instructions = expr->codegen_ir(ctx);
        instructions.emplace_back(codegen::ir::instruction{
            none, none,
            { boost::typeindex::type_id<codegen::ir::return_instruction>() },
            {},
            instructions.back().result
        });
        std::move(instructions.begin(), instructions.end(), std::back_inserter(statements));
        return unit{};
    });

    if (_is_top_level)
    {
        auto to_u32string = [](auto && v) {
            std::stringstream stream;
            stream << v;
            return boost::locale::conv::utf_to_utf<char32_t>(stream.str());
        };

        std::vector<codegen::ir::value> labeled_return_values;

        fmap(statements, [&](auto && stmt) {
            if (!stmt.instruction.template is<codegen::ir::return_instruction>())
            {
                return unit{};
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

            return unit{};
        });

        if (labeled_return_values.size() != 2)
        {
            statements.emplace_back(codegen::ir::instruction{
                optional<std::u32string>{ U"__return_phi" }, none,
                { boost::typeindex::type_id<codegen::ir::phi_instruction>() },
                std::move(labeled_return_values),
                codegen::ir::make_variable(return_type()->codegen_type(ctx))
            });

            fmap(statements, [&](auto && stmt) {
                if (!stmt.instruction.template is<codegen::ir::return_instruction>())
                {
                    return unit{};
                }

                stmt.instruction = boost::typeindex::type_id<codegen::ir::jump_instruction>();
                stmt.operands = { codegen::ir::boolean_value{ true }, codegen::ir::label{ U"__return_phi", {} } };

                return unit{};
            });

            statements.emplace_back(codegen::ir::instruction{
                none, none,
                { boost::typeindex::type_id<codegen::ir::return_instruction>() },
                {},
                statements.back().result
            });
        }
    }

    return statements;
}

reaver::vapor::codegen::_v1::ir::value reaver::vapor::analyzer::_v1::block::codegen_return(reaver::vapor::analyzer::_v1::ir_generation_context & ctx) const
{
    assert(_is_top_level);
    return codegen_ir(ctx).back().result;
}

