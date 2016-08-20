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

#include "../parser/lambda_expression.h"
#include "scope.h"
#include "statement.h"
#include "block.h"

namespace reaver
{
    namespace vapor
    {
        namespace analyzer { inline namespace _v1
        {
            class closure : public expression
            {
            public:
                closure(const parser::lambda_expression & parse, std::shared_ptr<scope> lex_scope) : _parse{ parse }, _scope{ std::make_shared<scope>(std::move(lex_scope)) }
                {
                    _body = preanalyze_block(*parse.body, _scope);
                }

                auto & parse() const
                {
                    return _parse;
                }

            private:
                virtual future<> _analyze() override
                {
                    return _body->analyze();
                }

                const parser::lambda_expression & _parse;
                std::shared_ptr<scope> _scope;
                std::shared_ptr<block> _body;
            };

            std::shared_ptr<closure> preanalyze_closure(const parser::lambda_expression & parse, std::shared_ptr<scope> lex_scope)
            {
                return std::make_shared<closure>(parse, std::move(lex_scope));
            }
        }}
    }
}

