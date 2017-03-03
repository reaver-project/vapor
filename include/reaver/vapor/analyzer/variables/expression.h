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

#include "variable.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    class expression_variable : public variable
    {
    public:
        expression_variable(std::unique_ptr<expression> expr, type * type) : _expression{ std::move(expr) }, _type{ type }
        {
        }

        virtual type * get_type() const override
        {
            return _type;
        }

        virtual bool is_constant() const override;
        virtual bool is_equal(const variable *) const override;

    private:
        virtual std::unique_ptr<variable> _clone_with_replacement(replacements &) const override;
        virtual future<variable *> _simplify(simplification_context & ctx) override;
        virtual variable_ir _codegen_ir(ir_generation_context &) const override;

        std::unique_ptr<expression> _expression;
        type * _type;
    };

    inline std::unique_ptr<variable> make_expression_variable(std::unique_ptr<expression> expr, type * type)
    {
        return std::make_unique<expression_variable>(std::move(expr), type);
    }

    class expression_ref_variable : public variable
    {
    public:
        expression_ref_variable(expression * expr, type * type) : _expression{ expr }, _type{ type }
        {
        }

        virtual type * get_type() const override
        {
            return _type;
        }

        virtual bool is_constant() const override;
        virtual bool is_equal(const variable *) const override;

    private:
        virtual std::unique_ptr<variable> _clone_with_replacement(replacements &) const override;
        virtual future<variable *> _simplify(simplification_context & ctx) override;
        virtual variable_ir _codegen_ir(ir_generation_context &) const override;

        expression * _expression;
        type * _type;
    };

    inline std::unique_ptr<variable> make_expression_ref_variable(expression * expr, type * type)
    {
        return std::make_unique<expression_ref_variable>(expr, type);
    }
}
}
