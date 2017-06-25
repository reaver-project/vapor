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

#include <memory>

#include "../../parser/literal.h"
#include "expression.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    class boolean_constant : public expression
    {
    public:
        boolean_constant(const parser::boolean_literal & parse) : _value{ parse.value.string == U"true" }
        {
            _parse.address = &parse;
            _parse.range = parse.range;
        }

        boolean_constant(bool value, synthesized_node<void> parse = {}) : _parse{ parse }, _value{ std::move(value) }
        {
        }

        virtual void print(std::ostream & os, print_context ctx) const override
        {
            os << styles::def << ctx << styles::rule_name << "boolean-constant";
            print_address_range(os, this);
            os << ' ' << styles::string_value << _value << '\n';
        }

        const auto & parse() const
        {
            return _parse;
        }

        auto get_value() const
        {
            return _value;
        }

    private:
        virtual future<> _analyze(analysis_context &) override
        {
            return make_ready_future();
        }

        virtual std::unique_ptr<expression> _clone_expr_with_replacement(replacements & repl) const override
        {
            return std::make_unique<boolean_constant>(_value, _parse);
        }

        virtual future<expression *> _simplify_expr(simplification_context &) override
        {
            return make_ready_future<expression *>(this);
        }

        virtual statement_ir _codegen_ir(ir_generation_context &) const override;

        virtual bool _is_equal(const expression * rhs) const override
        {
            auto rhs_int = dynamic_cast<const boolean_constant *>(rhs);
            return rhs_int && _value == rhs_int->_value;
        }

        synthesized_node<void> _parse;
        bool _value;
    };
}
}
