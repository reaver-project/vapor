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

#include "expression.h"
#include "variable.h"

namespace reaver::vapor::parser
{
inline namespace _v1
{
    struct member_expression;
}
}

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    class member_expression : public expression
    {
    public:
        member_expression(const parser::member_expression & parse);

        virtual void print(std::ostream & os, std::size_t indent) const override;

        virtual variable * get_variable() const override;

    private:
        member_expression(const parser::member_expression & parse, variable * referenced) : _parse{ parse }, _referenced{ referenced }
        {
        }

        virtual future<> _analyze(analysis_context &) override;

        virtual std::unique_ptr<expression> _clone_expr_with_replacement(replacements & repl) const override
        {
            if (_referenced)
            {
                return make_variable_expression(_referenced->clone_with_replacement(repl));
            }

            assert(!"tried to clone_expr_with_replacement a member expression that refers to a member assignment; this shouldn't've survived analysis!");
        }

        virtual future<expression *> _simplify_expr(simplification_context &) override
        {
            return make_ready_future<expression *>(this);
        }

        virtual statement_ir _codegen_ir(ir_generation_context &) const override;

        const parser::member_expression & _parse;

        variable * _referenced = nullptr;
        variable * _base = nullptr;
    };

    inline std::unique_ptr<member_expression> preanalyze_member_expression(const parser::member_expression & parse, scope *)
    {
        return std::make_unique<member_expression>(parse);
    }
}
}
