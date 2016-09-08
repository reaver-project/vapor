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

#include <numeric>

#include "../parser/id_expression.h"
#include "expression.h"
#include "symbol.h"

namespace reaver
{
    namespace vapor
    {
        namespace analyzer { inline namespace _v1
        {
            class id_expression : public expression
            {
            public:
                id_expression(const parser::id_expression & parse, std::shared_ptr<scope> lex_scope) : _parse{ parse }, _lex_scope{ lex_scope }
                {
                }

            private:
                virtual future<> _analyze() override
                {
                    return std::accumulate(_parse.id_expression_value.begin() + 1, _parse.id_expression_value.end(), _lex_scope->resolve(_parse.id_expression_value.front().string),
                        [&](auto fut, auto && ident) {
                            return fut.then([&ident](auto && symbol) {
                                return symbol->get_variable_future();
                            }).then([&ident](auto && var) {
                                return var->get_type()->get_scope()->get_future(ident.string);
                            });
                        }).then([](auto && symbol) {
                            return symbol->get_variable_future();
                        }).then([this](auto && variable) {
                            _set_variable(variable);
                        });
                }

                const parser::id_expression & _parse;
                std::shared_ptr<scope> _lex_scope;
            };

            inline std::shared_ptr<id_expression> preanalyze_id_expression(const parser::id_expression & parse, std::shared_ptr<scope> lex_scope)
            {
                return std::make_shared<id_expression>(parse, std::move(lex_scope));
            }
        }}
    }
}

