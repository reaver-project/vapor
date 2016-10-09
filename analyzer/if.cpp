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

#include "vapor/parser/lambda_expression.h"
#include "vapor/analyzer/symbol.h"
#include "vapor/analyzer/if.h"
#include "vapor/analyzer/helpers.h"
#include "vapor/analyzer/boolean.h"

void reaver::vapor::analyzer::_v1::if_statement::print(std::ostream & os, std::size_t indent) const
{
    auto in = std::string(indent, ' ');
    os << in << "if statement at " << _parse.range << '\n';
    os << in << "condition:\n";
    os << in << "{\n";
    _condition->print(os, indent + 4);
    os << in << "}\n";

    os << in << "then-block:\n";
    os << in << "{\n";
    _then_block->print(os, indent + 4);
    os << in << "}\n";

    fmap(_else_block, [&](auto && block) {
        os << in << "else-block:\n";
        os << in << "{\n";
        block->print(os, indent + 4);
        os << in << "}\n";
        return unit{};
    });
}

reaver::future<> reaver::vapor::analyzer::_v1::if_statement::_analyze()
{
    // I'd write this with when_all, but first I need to fix that
    // for empty tuples (or just make tuples "special", like David Sankel suggested)

    auto fut = _condition->analyze()
        .then([&]() {
            return _then_block->analyze();
        });

    fmap(_else_block, [&](auto && else_) {
        fut = fut.then([&]{ return else_->analyze(); });
        return unit{};
    });

    return fut;
}

reaver::future<reaver::vapor::analyzer::_v1::statement *> reaver::vapor::analyzer::_v1::if_statement::_simplify(reaver::vapor::analyzer::_v1::optimization_context & ctx)
{
    return _condition->simplify_expr(ctx)
        .then([&](auto && simplified) {
            replace_uptr(_condition, simplified, ctx);

            auto var = _condition->get_variable();
            if (var->is_constant())
            {
                if (var->get_type() != builtin_types().boolean.get())
                {
                    assert(0);
                }

                auto condition = dynamic_cast<boolean_constant *>(var)->get_value();
                if (condition)
                {
                    return _then_block.release()->simplify(ctx);
                }

                else
                {
                    if (_else_block)
                    {
                        return _else_block.get().release()->simplify(ctx);
                    }

                    return make_null_statement().release()->simplify(ctx);
                }
            }

            return make_ready_future<statement *>(this);
        });
}

reaver::vapor::analyzer::_v1::statement_ir reaver::vapor::analyzer::_v1::if_statement::_codegen_ir(reaver::vapor::analyzer::_v1::ir_generation_context & ctx) const
{
    // instructions returning those labels are silly
    // really need to do this better at some point
    // (future refactor perhaps)
    // (perhaps once I have tuples, so they can return unit type values)

    auto condition_instructions = _condition->codegen_ir(ctx);
    auto condition_variable = condition_instructions.back().result;

    auto else_label = U"__else" + boost::locale::conv::utf_to_utf<char32_t>(std::to_string(ctx.label_index++));

    auto then_instructions = _then_block->codegen_ir(ctx);
    auto else_instructions = _else_block
        ? [&]() {
            auto ir = _else_block.get()->codegen_ir(ctx);
            ir.front().label = else_label;
            return ir;
        }()
        : statement_ir{
            codegen::ir::instruction{
                else_label, {},
                boost::typeindex::type_id<codegen::ir::noop_instruction>(),
                {}, codegen::ir::label{ else_label, {} }
            }
        };

    auto jump = codegen::ir::instruction{
        {}, {},
        boost::typeindex::type_id<codegen::ir::jump_instruction>(),
        { condition_variable, codegen::ir::label{ else_label, {} } },
        codegen::ir::label{ else_label, {} }
    };

    statement_ir ret;
    ret.reserve(condition_instructions.size() + 1 + then_instructions.size() + else_instructions.size());
    std::move(condition_instructions.begin(), condition_instructions.end(), std::back_inserter(ret));
    ret.push_back(std::move(jump));
    std::move(then_instructions.begin(), then_instructions.end(), std::back_inserter(ret));
    std::move(else_instructions.begin(), else_instructions.end(), std::back_inserter(ret));
    return ret;
}

