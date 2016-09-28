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

#include <boost/multiprecision/integer.hpp>

#include "literal.h"
#include "../parser/literal.h"
#include "expression.h"
#include "function.h"
#include "../codegen/ir/integer.h"

namespace reaver
{
    namespace vapor
    {
        namespace analyzer { inline namespace _v1
        {
            class integer_type : public type
            {
            public:
                virtual std::shared_ptr<function> get_overload(lexer::token_type token, std::shared_ptr<type> rhs) const override
                {
                    switch (token)
                    {
                        case lexer::token_type::plus:
                            return _addition();

                        case lexer::token_type::star:
                            return _multiplication();

                        default:
                            assert(!"unimplemented int op");
                    }
                }

                virtual std::string explain() const override
                {
                    return "integer";
                }

            private:
                virtual std::shared_ptr<codegen::ir::variable_type> _codegen_type(ir_generation_context &) const override;

                // throw all this shit into a .cpp file
                template<typename Instruction, typename Eval>
                static auto _generate_function(const char32_t * name, Eval eval);
                static std::shared_ptr<function> _addition();
                static std::shared_ptr<function> _multiplication();
            };

            class integer_constant : public literal
            {
            public:
                integer_constant(const parser::integer_literal & parse) : _value{ utf8(parse.value.string) }
                {
                }

                integer_constant(boost::multiprecision::cpp_int value) : _value{ std::move(value) }
                {
                }

                virtual std::shared_ptr<type> get_type() const override
                {
                    return builtin_types().integer;
                }

                auto get_value() const
                {
                    return _value;
                }

                virtual bool is_constant() const override
                {
                    return true;
                }

                virtual bool is_equal(std::shared_ptr<const variable> other_var) const override
                {
                    auto other = std::dynamic_pointer_cast<const integer_constant>(other_var);
                    if (!other)
                    {
                        // todo: conversions somehow
                        return false;
                    }

                    return other->get_value() == _value;
                }

            private:
                virtual variable_ir _codegen_ir(ir_generation_context &) const override;

                boost::multiprecision::cpp_int _value;
            };

            class integer_literal : public expression
            {
            public:
                integer_literal(const parser::integer_literal & parse) : _parse{ parse }, _value{ std::make_shared<integer_constant>(parse) }
                {
                    _set_variable(_value);
                }

                virtual void print(std::ostream & os, std::size_t indent) const override
                {
                    auto in = std::string(indent, ' ');
                    os << in << "integer literal with value of " << _value->get_value() << " at " << _parse.range << '\n';
                }

            private:
                virtual future<> _analyze() override
                {
                    return make_ready_future();
                }

                virtual future<std::shared_ptr<expression>> _simplify_expr(optimization_context &) override
                {
                    return make_ready_future(_shared_from_this());
                }

                virtual statement_ir _codegen_ir(ir_generation_context &) const override;

                const parser::integer_literal & _parse;
                std::shared_ptr<integer_constant> _value;
            };

            inline std::shared_ptr<type> make_integer_type()
            {
                static auto int_t = std::make_shared<integer_type>();
                return int_t;
            }
        }}
    }
}

