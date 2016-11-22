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

#pragma once

#include "../parser/if_statement.h"
#include "block.h"
#include "statement.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    class if_statement : public statement
    {
    public:
        if_statement(const parser::if_statement & parse, scope * lex_scope) : _parse{ parse }
        {
            _condition = preanalyze_expression(parse.condition, lex_scope);
            _then_block = preanalyze_block(parse.then_block, lex_scope, false);
            _else_block = fmap(parse.else_block, [&](auto && parse) { return preanalyze_block(parse, lex_scope, false); });
        }

        virtual std::vector<const return_statement *> get_returns() const override
        {
            std::vector<block *> blocks{ _then_block.get() };
            fmap(_else_block, [&](auto && block) {
                blocks.push_back(block.get());
                return unit{};
            });
            return mbind(blocks, [&](auto && block) { return block->get_returns(); });
        }

        virtual void print(std::ostream & os, std::size_t indent) const override;

    private:
        if_statement(const if_statement & other) : _parse{ other._parse }
        {
        }

        virtual future<> _analyze(analysis_context &) override;
        virtual std::unique_ptr<statement> _clone_with_replacement(replacements &) const override;
        virtual future<statement *> _simplify(simplification_context &) override;
        virtual statement_ir _codegen_ir(ir_generation_context &) const override;

        const parser::if_statement & _parse;

        std::unique_ptr<expression> _condition;
        std::unique_ptr<block> _then_block;
        optional<std::unique_ptr<block>> _else_block;
    };

    inline std::unique_ptr<if_statement> preanalyze_if_statement(const parser::if_statement & parse, scope * lex_scope)
    {
        return std::make_unique<if_statement>(parse, lex_scope);
    }
}
}
