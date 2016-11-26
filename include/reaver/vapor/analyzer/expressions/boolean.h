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
#include "../function.h"
#include "../variables/boolean.h"
#include "variable.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
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
