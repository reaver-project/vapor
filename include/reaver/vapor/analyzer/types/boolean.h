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

#include "../../codegen/ir/boolean.h"
#include "../../parser/literal.h"
#include "../expressions/expression.h"
#include "../function.h"
#include "../variables/literal.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    class boolean_type : public type
    {
    public:
        virtual future<function *> get_overload(lexer::token_type token, const type * rhs) const override
        {
            switch (token)
            {
                case lexer::token_type::equals:
                    return make_ready_future(_equal_comparison());

                default:
                    assert(!"unimplemented int op");
            }
        }

        virtual std::string explain() const override
        {
            return "boolean";
        }

    private:
        virtual void _codegen_type(ir_generation_context &) const override;

        template<typename Instruction, typename Eval>
        static auto _generate_function(const char32_t * name, const char * desc, Eval eval, type * return_type);
        static function * _equal_comparison();
    };

    class boolean_constant : public literal
    {
    public:
        boolean_constant(const parser::boolean_literal & parse) : _value{ parse.value.string == U"true" }
        {
        }

        boolean_constant(bool value) : _value{ value }
        {
        }

        virtual type * get_type() const override
        {
            return builtin_types().boolean.get();
        }

        auto get_value() const
        {
            return _value;
        }

        virtual bool is_constant() const override
        {
            return true;
        }

        virtual bool is_equal(const variable * other_var) const override
        {
            auto other = dynamic_cast<const boolean_constant *>(other_var);
            if (!other)
            {
                // todo: conversions somehow
                return false;
            }

            return other->get_value() == _value;
        }

    private:
        virtual std::unique_ptr<variable> _clone_with_replacement(replacements &) const override
        {
            return std::make_unique<boolean_constant>(_value);
        }

        virtual variable_ir _codegen_ir(ir_generation_context &) const override;

        bool _value;
    };

    class boolean_literal : public expression
    {
    public:
        boolean_literal(const parser::boolean_literal & parse) : _parse{ parse }
        {
            auto val = std::make_unique<boolean_constant>(parse);
            _value = val.get();
            _set_variable(std::move(val));
        }

        virtual void print(std::ostream & os, std::size_t indent) const override
        {
            auto in = std::string(indent, ' ');
            os << in << "boolean literal with value of " << _value->get_value() << " at " << _parse.range << '\n';
        }

    private:
        virtual future<> _analyze(analysis_context &) override
        {
            return make_ready_future();
        }

        virtual std::unique_ptr<expression> _clone_expr_with_replacement(replacements & repl) const override
        {
            return make_variable_expression(_value->clone_with_replacement(repl));
        }

        virtual future<expression *> _simplify_expr(simplification_context &) override
        {
            return make_ready_future<expression *>(this);
        }

        virtual statement_ir _codegen_ir(ir_generation_context &) const override;

        const parser::boolean_literal & _parse;
        boolean_constant * _value;
    };

    std::unique_ptr<type> make_boolean_type();
}
}
