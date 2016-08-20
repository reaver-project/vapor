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

#include "../parser/expression.h"
#include "../parser/expression_list.h"
#include "../parser/lambda_expression.h"
#include "scope.h"
#include "helpers.h"
#include "variable.h"
#include "statement.h"

namespace reaver
{
    namespace vapor
    {
        namespace analyzer { inline namespace _v1
        {
            class expression : public statement, public std::enable_shared_from_this<expression>
            {
            public:
                expression() = default;
                virtual ~expression() = default;

                expression(std::shared_ptr<variable> var) : _variable{ std::move(var) }
                {
                }

                /*virtual void analyze() override
                {
                    throw std::runtime_error{ typeid(this).name() };
                }*/

                std::shared_ptr<variable> get_variable() const
                {
                    return _variable;
                }

                std::shared_ptr<type> get_type()
                {
                    if (!_variable)
                    {
                        assert(!"someone tried to get type before analyzing... or forgot to set variable from analyze");
                    }

                    return _variable->get_type();
                }

            protected:
                void _set_variable(std::shared_ptr<variable> var)
                {
                    assert(var);
                    assert(!_variable);
                    _variable = var;
                }

            private:
                std::shared_ptr<variable> _variable;
            };

            class expression_list : public expression
            {
            private:
                virtual future<> _analyze() override
                {
                    return when_all(fmap(value, [&](auto && expr) { return expr->analyze(); }))
                        .then([&]{ _set_variable(value.back()->get_variable()); });
                }

            public:
                range_type range;
                std::vector<std::shared_ptr<expression>> value;
            };

            std::shared_ptr<expression> preanalyze_expression(const parser::expression & expr, const std::shared_ptr<scope> & lex_scope);

            inline std::shared_ptr<expression> preanalyze_expression(const parser::expression_list & expr, const std::shared_ptr<scope> & lex_scope)
            {
                if (expr.expressions.size() > 1)
                {
                    return preanalyze_expression(expr, lex_scope);
                }

                auto ret = std::make_shared<expression_list>();
                ret->value.reserve(expr.expressions.size());
                std::transform(expr.expressions.begin(), expr.expressions.end(), std::back_inserter(ret->value), [&](auto && expr)
                {
                    return preanalyze_expression(expr, lex_scope);
                });
                return ret;
            }
        }}
    }
}

