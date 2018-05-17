/**
 * Vapor Compiler Licence
 *
 * Copyright © 2016-2018 Michał "Griwes" Dominiak
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
    struct precontext;

    class block;
    std::unique_ptr<block> preanalyze_block(precontext &, const parser::block &, scope *, bool);

    class return_statement;
    class scope;

    class block : public statement
    {
    public:
        block(ast_node parse,
            std::unique_ptr<scope> lex_scope,
            scope * original_scope,
            std::vector<std::unique_ptr<statement>> statements,
            std::optional<std::unique_ptr<expression>> value_expr,
            bool is_top_level);

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
            return _value_expr.has_value();
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

        codegen::ir::value codegen_return(ir_generation_context &) const;

    private:
        block(const block & other) : _original_scope{ other._original_scope }, _is_top_level{ other._is_top_level }
        {
        }

        virtual future<> _analyze(analysis_context &) override;
        virtual std::unique_ptr<statement> _clone_with_replacement(replacements &) const override;
        virtual future<statement *> _simplify(recursive_context) override;
        virtual statement_ir _codegen_ir(ir_generation_context &) const override;

        std::unique_ptr<scope> _scope;
        const scope * const _original_scope;
        std::vector<std::unique_ptr<statement>> _statements;
        std::optional<std::unique_ptr<expression>> _value_expr;
        const bool _is_top_level = false;

        void _ensure_cache() const;

        mutable std::mutex _clone_cache_lock;
        bool _is_clone_cache = false;
        mutable std::optional<std::unique_ptr<block>> _clone;
    };

    std::unique_ptr<block> preanalyze_block(precontext & ctx, const parser::block & parse, scope * lex_scope, bool is_top_level);
}
}
