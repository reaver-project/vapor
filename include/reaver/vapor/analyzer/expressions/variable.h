/**
 * Vapor Compiler Licence
 *
 * Copyright © 2014-2016 Michał "Griwes" Dominiak
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
#include "../variables/variable.h"
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

    class variable_ref_expression : public expression
    {
    public:
        variable_ref_expression(variable * var) : _referenced(var)
        {
            assert(var);
        }

        virtual variable * get_variable() const final override
        {
            return _referenced;
        }

        virtual void print(std::ostream & os, std::size_t indent) const override;

    private:
        virtual future<> _analyze(analysis_context &) override
        {
            return make_ready_future();
        }

        virtual std::unique_ptr<expression> _clone_expr_with_replacement(replacements & repl) const override
        {
            auto it = repl.variables.find(_referenced);
            if (it == repl.variables.end())
            {
                return std::make_unique<variable_ref_expression>(_referenced);
            }

            return std::make_unique<variable_ref_expression>(it->second);
        }

        virtual future<expression *> _simplify_expr(simplification_context & ctx) override
        {
            return _referenced->simplify(ctx).then([&](auto && simplified) -> expression * {
                _referenced = simplified;
                return this;
            });
        }

        virtual statement_ir _codegen_ir(ir_generation_context & ctx) const override
        {
            return { codegen::ir::instruction{
                none, none, { boost::typeindex::type_id<codegen::ir::pass_value_instruction>() }, {}, get<codegen::ir::value>(_referenced->codegen_ir(ctx)) } };
        }

        variable * _referenced;
    };

    inline std::unique_ptr<expression> make_variable_ref_expression(variable * var)
    {
        return std::make_unique<variable_ref_expression>(var);
    }

    class variable_expression : public expression
    {
    public:
        variable_expression(std::unique_ptr<variable> var)
        {
            _set_variable(std::move(var));
        }

        virtual void print(std::ostream & os, std::size_t indent) const override;

    private:
        virtual future<> _analyze(analysis_context &) override
        {
            return make_ready_future();
        }

        std::unique_ptr<expression> _clone_expr_with_replacement(replacements & repl) const override
        {
            auto it = repl.variables.find(get_variable());
            if (it == repl.variables.end())
            {
                return std::make_unique<variable_expression>(get_variable()->clone_with_replacement(repl));
            }

            return make_variable_ref_expression(it->second);
        }

        virtual future<expression *> _simplify_expr(simplification_context & ctx) override
        {
            return get_variable()->simplify(ctx).then([&](auto && simplified) -> expression * {
                this->_set_variable(simplified, ctx);
                return this;
            });
        }

        virtual statement_ir _codegen_ir(ir_generation_context & ctx) const override
        {
            return { codegen::ir::instruction{ none,
                none,
                { boost::typeindex::type_id<codegen::ir::pass_value_instruction>() },
                {},
                get<codegen::ir::value>(get_variable()->codegen_ir(ctx)) } };
        }
    };

    inline std::unique_ptr<expression> make_variable_expression(std::unique_ptr<variable> var)
    {
        return std::make_unique<variable_expression>(std::move(var));
    }
}
}
