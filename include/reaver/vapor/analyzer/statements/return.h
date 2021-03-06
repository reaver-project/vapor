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

#include "../expressions/expression.h"
#include "../helpers.h"
#include "../statements/statement.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    class return_statement : public statement
    {
    public:
        return_statement(ast_node parse, std::unique_ptr<expression> value);

        virtual std::vector<const return_statement *> get_returns() const override
        {
            return { this };
        }

        type * get_returned_type() const
        {
            return _value_expr->get_type();
        }

        expression * get_returned_expression() const
        {
            return _value_expr.get();
        }

        virtual bool always_returns() const override
        {
            return true;
        }

        virtual void print(std::ostream & os, print_context) const override;

    private:
        virtual future<> _analyze(analysis_context & ctx) override
        {
            return _value_expr->analyze(ctx);
        }

        return_statement(const return_statement & other)
        {
        }

        virtual std::unique_ptr<statement> _clone(replacements & repl) const override
        {
            auto ret = std::unique_ptr<return_statement>(new return_statement(*this));
            ret->_value_expr = repl.claim(_value_expr.get());
            return ret;
        }

        virtual future<statement *> _simplify(recursive_context ctx) override
        {
            return _value_expr->simplify_expr(ctx).then([&, ctx](auto && simplified) -> statement * {
                replace_uptr(_value_expr, simplified, ctx.proper);
                return this;
            });
        }

        virtual statement_ir _codegen_ir(ir_generation_context &) const override;

        std::unique_ptr<expression> _value_expr;
    };
}
}

namespace reaver::vapor::parser
{
inline namespace _v1
{
    struct return_expression;
}
}

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    struct precontext;

    std::unique_ptr<return_statement> preanalyze_return(precontext & cts,
        const parser::return_expression & parse,
        scope * lex_scope);
}
}
