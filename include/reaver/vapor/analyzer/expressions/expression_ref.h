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

#include "expression.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    class expression_ref : public expression
    {
    protected:
        expression_ref() = default;

    public:
        expression_ref(expression * expr) : expression{ expr->get_type() }, _referenced{ expr }
        {
        }

        virtual void print(std::ostream & os, print_context ctx) const override
        {
            os << styles::def << ctx << styles::rule_name << "expression-ref";
            os << styles::def << " @ " << styles::address << this << styles::def << ":\n";
            _referenced->print(os, ctx.make_branch(true));
        }

    private:
        virtual expression * _get_replacement() override
        {
            return _referenced->_get_replacement();
        }

        virtual const expression * _get_replacement() const override
        {
            return _referenced->_get_replacement();
        }

        virtual std::unique_ptr<expression> _clone_expr_with_replacement(replacements & repl) const override
        {
            auto referenced = _referenced;

            auto it = repl.expressions.find(referenced);
            if (it != repl.expressions.end())
            {
                referenced = it->second;
            }

            return std::make_unique<expression_ref>(referenced);
        }

        virtual future<expression *> _simplify_expr(simplification_context & ctx) override
        {
            return _referenced->simplify_expr(ctx).then([&](auto && simplified) -> expression * {
                if (simplified && simplified != _referenced)
                {
                    _referenced = simplified;
                }
                return this;
            });
        }

        virtual statement_ir _codegen_ir(ir_generation_context & ctx) const override
        {
            auto referenced_ir = _referenced->codegen_ir(ctx);
            return { codegen::ir::instruction{
                none, none, { boost::typeindex::type_id<codegen::ir::pass_value_instruction>() }, {}, { referenced_ir.back().result } } };
        }

        virtual bool _is_equal(const expression * rhs) const override
        {
            return _referenced->is_equal(rhs);
        }

    protected:
        expression * _referenced = nullptr;
    };

    inline std::unique_ptr<expression> make_expression_ref(expression * expr)
    {
        return std::make_unique<expression_ref>(expr);
    }
}
}
