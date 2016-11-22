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

#include <memory>
#include <numeric>

#include "../parser/lambda_expression.h"
#include "argument_list.h"
#include "block.h"
#include "function.h"
#include "scope.h"
#include "statement.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    class closure_type : public type
    {
    public:
        closure_type(scope * lex_scope, expression * closure, std::unique_ptr<function> fn)
            : type{ lex_scope }, _closure{ std::move(closure) }, _function{ std::move(fn) }
        {
        }

        virtual std::string explain() const override
        {
            return "closure (TODO: location)";
        }

        virtual future<function *> get_overload(lexer::token_type bracket, std::vector<const type *> args) const override
        {
            if (args.size() == _function->arguments().size()
                && std::inner_product(args.begin(), args.end(), _function->arguments().begin(), true, std::logical_and<>(), [](auto && type, auto && var) {
                       return type == var->get_type();
                   }))
            {
                return make_ready_future(_function.get());
            }

            return make_ready_future(static_cast<function *>(nullptr));
        }

    private:
        virtual void _codegen_type(ir_generation_context &) const override;

        expression * _closure;
        std::unique_ptr<function> _function;
    };

    class closure : public expression
    {
    public:
        closure(const parser::lambda_expression & parse, scope * lex_scope) : _parse{ parse }, _scope{ lex_scope->clone_local() }
        {
            fmap(parse.arguments, [&](auto && arglist) {
                _argument_list = preanalyze_argument_list(arglist, _scope.get());
                return unit{};
            });
            _scope->close();

            _return_type = fmap(_parse.return_type, [&](auto && ret_type) { return preanalyze_expression(ret_type, _scope.get()); });
            _body = preanalyze_block(parse.body, _scope.get(), true);
        }

        auto & parse() const
        {
            return _parse;
        }

        virtual void print(std::ostream & os, std::size_t indent) const override;

    private:
        virtual future<> _analyze(analysis_context &) override;
        virtual std::unique_ptr<expression> _clone_expr_with_replacement(replacements &) const override;
        virtual future<expression *> _simplify_expr(simplification_context &) override;
        virtual statement_ir _codegen_ir(ir_generation_context &) const override;

        const parser::lambda_expression & _parse;
        argument_list _argument_list;

        optional<std::unique_ptr<expression>> _return_type;
        std::unique_ptr<scope> _scope;
        std::unique_ptr<block> _body;
        std::unique_ptr<type> _type;
    };

    inline std::unique_ptr<closure> preanalyze_closure(const parser::lambda_expression & parse, scope * lex_scope)
    {
        return std::make_unique<closure>(parse, lex_scope);
    }
}
}
