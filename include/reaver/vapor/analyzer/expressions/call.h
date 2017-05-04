/**
 * Vapor Compiler Licence
 *
 * Copyright © 2017 Michał "Griwes" Dominiak
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

#include "../../range.h"
#include "../function.h"
#include "expression.h"
#include "variable.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    class call_expression : public expression
    {
    public:
        call_expression(function * fun, std::vector<expression *> args) : _function{ fun }, _args{ std::move(args) }
        {
        }

        void set_parse_range(const range_type & range)
        {
            assert(!_range);
            _range = range;
        }

        virtual void print(std::ostream &, std::size_t indent) const override;
        virtual variable * get_variable() const override;

        void replace_with(std::unique_ptr<expression> expr)
        {
            assert(!_var);
            assert(!_replacement_expr);

            _replacement_expr = std::move(expr);
        }

        const range_type & get_range() const
        {
            assert(_range);
            return *_range;
        }

    private:
        virtual future<> _analyze(analysis_context &) override;
        virtual std::unique_ptr<expression> _clone_expr_with_replacement(replacements & repl) const override;
        virtual future<expression *> _simplify_expr(simplification_context &) override;
        virtual statement_ir _codegen_ir(ir_generation_context &) const override;

    protected:
        function * _function;
        optional<const range_type &> _range;
        std::unique_ptr<expression> _replacement_expr;

    private:
        std::vector<expression *> _args;
        std::unique_ptr<variable> _var;
        std::unique_ptr<expression> _cloned_type_expr;
    };

    class owning_call_expression : public call_expression
    {
    public:
        owning_call_expression(function * fun, std::vector<std::unique_ptr<expression>> args)
            : call_expression{ fun, fmap(args, [](auto && arg) { return arg.get(); }) }, _var_exprs{ std::move(args) }
        {
        }

    private:
        virtual future<expression *> _simplify_expr(simplification_context &) override;

        std::vector<std::unique_ptr<expression>> _var_exprs;
    };

    inline auto make_call_expression(function * fun, std::vector<expression *> args)
    {
        return std::make_unique<call_expression>(fun, args);
    }

    inline auto make_call_expression(function * fun, std::vector<variable *> args)
    {
        return std::make_unique<owning_call_expression>(fun, fmap(args, [](auto && arg) { return make_variable_ref_expression(arg); }));
    }
}
}
