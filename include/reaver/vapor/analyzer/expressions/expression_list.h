/**
 * Vapor Compiler Licence
 *
 * Copyright © 2014-2019 Michał "Griwes" Dominiak
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

#include <reaver/prelude/monad.h>

#include "../helpers.h"
#include "expression.h"

namespace reaver::vapor::parser
{
inline namespace _v1
{
    struct expression;
    struct expression_list;
}
}

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    class scope;
    struct precontext;

    class expression_list : public expression
    {
    private:
        virtual future<> _analyze(analysis_context &) override;
        virtual std::unique_ptr<expression> _clone_expr(replacements &) const override;
        virtual future<expression *> _simplify_expr(recursive_context) override;

        virtual statement_ir _codegen_ir(ir_generation_context & ctx) const override
        {
            return mbind(value, [&](auto && expr) { return expr->codegen_ir(ctx); });
        }

        virtual constant_init_ir _constinit_ir(ir_generation_context &) const override
        {
            assert(0);
        }

        virtual bool _is_equal(const expression * rhs) const override
        {
            return value.back()->is_equal(rhs);
        }

        virtual std::unique_ptr<google::protobuf::Message> _generate_interface() const override
        {
            assert(0);
        }

    public:
        expression_list(ast_node node)
        {
            _set_ast_info(node);
        }

        virtual void print(std::ostream & os, print_context ctx) const override;

        virtual bool is_constant() const override
        {
            return std::all_of(value.begin(), value.end(), [](auto && expr) { return expr->is_constant(); });
        }

        friend std::unique_ptr<expression> preanalyze_expression_list(precontext &,
            const parser::expression_list &,
            scope *);

        std::vector<std::unique_ptr<expression>> value;
    };

    std::unique_ptr<expression> preanalyze_expression_list(precontext & ctx,
        const parser::expression_list & expr,
        scope * lex_scope);
}
}
