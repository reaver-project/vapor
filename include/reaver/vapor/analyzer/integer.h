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
                template<typename Instruction>
                static auto _generate_function(const char32_t * name)
                {
                    return make_function(
                        "<builtin integer addition>",
                        builtin_types().integer,
                        { builtin_types().integer, builtin_types().integer },
                        [name](ir_generation_context & ctx) {
                            auto lhs = codegen::ir::make_variable(
                                builtin_types().integer->codegen_type(ctx)
                            );
                            auto rhs = codegen::ir::make_variable(
                                builtin_types().integer->codegen_type(ctx)
                            );

                            auto retval = codegen::ir::make_variable(
                                builtin_types().integer->codegen_type(ctx)
                            );

                            return codegen::ir::function{
                                name,
                                {}, { lhs, rhs },
                                retval,
                                {
                                    codegen::ir::instruction{
                                        none, none,
                                        { boost::typeindex::type_id<Instruction>() },
                                        { lhs, rhs },
                                        retval
                                    },
                                    codegen::ir::instruction{
                                        none, none,
                                        { boost::typeindex::type_id<codegen::ir::return_instruction>() },
                                        {},
                                        retval
                                    }
                                }
                            };
                        }
                    );
                }

                static std::shared_ptr<function> _addition()
                {
                    static auto addition = _generate_function<codegen::ir::integer_addition_instruction>(U"__builtin_integer_operator_plus");
                    return addition;
                }

                static std::shared_ptr<function> _multiplication()
                {
                    static auto multiplication = _generate_function<codegen::ir::integer_multiplication_instruction>(U"__builtin_integer_operator_star");
                    return multiplication;
                }
            };

            class integer_constant : public literal
            {
            public:
                integer_constant(const parser::integer_literal & parse) : _value{ utf8(parse.value.string) }
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

