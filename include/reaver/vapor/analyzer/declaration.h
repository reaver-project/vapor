/**
 * Vapor Compiler Licence
 *
 * Copyright © 2014 Michał "Griwes" Dominiak
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

#include "vapor/parser/declaration.h"
#include "vapor/analyzer/symbol.h"
#include "vapor/analyzer/expression.h"

namespace reaver
{
    namespace vapor
    {
        namespace analyzer { inline namespace _v1
        {
            class declaration
            {
            public:
                declaration(std::u32string name, expression init, const std::shared_ptr<scope> & lex_scope, const parser::declaration & parse)
                    : _name{ std::move(name) }, _initializer_expression{ std::move(init) }, _parse{ parse }
                {
                    assert(0);
                }

                const auto & name() const
                {
                    return _name;
                }

                const auto & parse() const
                {
                    return _parse;
                }

                auto declared_symbol() const
                {
                    return _declared_symbol;
                }

                auto initializer_expression() const
                {
                    return _initializer_expression;
                }

            private:
                std::u32string _name;
                std::shared_ptr<symbol> _declared_symbol;
                expression _initializer_expression;

                const parser::declaration & _parse;
            };

            std::shared_ptr<declaration> make_declaration(std::u32string name, expression init, const std::shared_ptr<scope> & lexical_scope, const parser::declaration & parse)
            {
                return std::make_shared<declaration>(std::move(name), std::move(init), std::move(lexical_scope), parse);
            }
        }}
    }
}
