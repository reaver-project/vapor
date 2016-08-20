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

#include "../parser/postfix_expression.h"
#include "expression.h"

namespace reaver
{
    namespace vapor
    {
        namespace analyzer { inline namespace _v1
        {
            class postfix_expression : public expression
            {
            public:
                postfix_expression(const parser::postfix_expression & parse, std::shared_ptr<scope> lex_scope) : _parse{ parse }
                {
                }

            private:
                virtual future<> _analyze() override
                {
                    assert(0);
                }

                const parser::postfix_expression & _parse;
            };

            std::shared_ptr<postfix_expression> preanalyze_postfix_expression(const parser::postfix_expression & parse, std::shared_ptr<scope> lex_scope)
            {
                return std::make_shared<postfix_expression>(parse, std::move(lex_scope));
            }
        }}
    }
}

