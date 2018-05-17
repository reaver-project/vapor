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

#include "expression.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    class unary_expression : public expression
    {
    public:
        virtual void print(std::ostream &, print_context) const override
        {
            assert(0);
        }

    private:
        virtual future<> _analyze(analysis_context &) override
        {
            assert(0);
        }

        virtual std::unique_ptr<expression> _clone_expr_with_replacement(replacements &) const override
        {
            assert(0);
        }

        virtual future<expression *> _simplify_expr(recursive_context) override
        {
            assert(0);
        }

        virtual statement_ir _codegen_ir(ir_generation_context &) const override
        {
            assert(0);
        }
    };
}
}

namespace reaver::vapor::parser
{
inline namespace _v1
{
    struct unary_expression;
}
}

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    struct precontext;

    inline std::unique_ptr<unary_expression> preanalyze_unary_expression(precontext & ctx, const parser::unary_expression &, scope *)
    {
        assert(0);
    }
}
}
