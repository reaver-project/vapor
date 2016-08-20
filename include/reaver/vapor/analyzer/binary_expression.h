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

#include "../parser/binary_expression.h"
#include "expression.h"
#include "function.h"

namespace reaver
{
    namespace vapor
    {
        namespace analyzer { inline namespace _v1
        {
            class binary_expression : public expression
            {
            public:
                binary_expression(const parser::binary_expression & parse, std::shared_ptr<scope> lex_scope) : _parse{ parse }, _scope{ lex_scope }, _op{ _parse.op },
                    _lhs{ preanalyze_expression(_parse.lhs, lex_scope) },
                    _rhs{ preanalyze_expression(_parse.rhs, lex_scope) }
                {
                }

            private:
                virtual future<> _analyze() override
                {
                    return when_all(
                        _lhs->analyze(),
                        _rhs->analyze()
                    ).then([&](auto &&) {
                        auto _overload = resolve_overload(_lhs->get_type(), _rhs->get_type(), _op.type, _scope);
                        assert(_overload);

                        _set_variable(make_expression_variable(shared_from_this(), _overload->return_type()));
                    });
                }

                const parser::binary_expression & _parse;
                std::shared_ptr<scope> _scope;
                lexer::token _op;
                std::shared_ptr<expression> _lhs;
                std::shared_ptr<expression> _rhs;
                std::shared_ptr<function> _overload;
            };

            std::shared_ptr<binary_expression> preanalyze_binary_expression(const parser::binary_expression & parse, std::shared_ptr<scope> lex_scope)
            {
                return std::make_shared<binary_expression>(parse, std::move(lex_scope));
            }
        }}
    }
}

