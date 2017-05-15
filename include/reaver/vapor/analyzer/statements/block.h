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

#pragma once

#include <reaver/prelude/monad.h>

#include "../../codegen/ir/variable.h"
#include "../expressions/expression.h"
#include "statement.h"

namespace reaver::vapor::parser
{
inline namespace _v1
{
    struct block;
}
}

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    class block;
    std::unique_ptr<block> preanalyze_block(const parser::block &, scope *, bool);

    class return_statement;
    class scope;

    class block : public statement
    {
    public:
        block(const parser::block & parse, scope * lex_scope, bool is_top_level);

        type * value_type() const
        {
            if (_value_expr)
            {
                return (*_value_expr)->get_type();
            }

            return nullptr;
        }

        type * return_type() const;

        virtual std::vector<const return_statement *> get_returns() const override
        {
            return mbind(_statements, [](auto && stmt) { return stmt->get_returns(); });
        }

        bool has_return_expression() const
        {
            return _value_expr;
        }

        expression * get_return_expression() const
        {
            assert(_value_expr);
            return _value_expr->get();
        }

        virtual bool always_returns() const override
        {
            return !_statements.empty() && _statements.back()->always_returns();
        }

        virtual void print(std::ostream & os, print_context) const override;

        const auto & parse() const
        {
            return _parse;
        }

        codegen::ir::value codegen_return(ir_generation_context &) const;

    private:
        block(const block & other) : _parse{ other._parse }, _original_scope{ other._original_scope }, _is_top_level{ other._is_top_level }
        {
        }

        virtual future<> _analyze(analysis_context &) override;
        virtual std::unique_ptr<statement> _clone_with_replacement(replacements &) const override;
        virtual future<statement *> _simplify(simplification_context &) override;
        virtual statement_ir _codegen_ir(ir_generation_context &) const override;

        const parser::block & _parse;
        std::unique_ptr<scope> _scope;
        const scope * const _original_scope;
        std::vector<std::unique_ptr<statement>> _statements;
        optional<std::unique_ptr<expression>> _value_expr;
        const bool _is_top_level = false;

        void _ensure_cache() const;

        mutable std::mutex _clone_cache_lock;
        bool _is_clone_cache = false;
        mutable optional<std::unique_ptr<block>> _clone;
    };

    inline std::unique_ptr<block> preanalyze_block(const parser::block & parse, scope * lex_scope, bool is_top_level)
    {
        return std::make_unique<block>(parse, std::move(lex_scope), is_top_level);
    }
}
}
