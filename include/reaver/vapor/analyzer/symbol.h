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

#include "vapor/analyzer/scope.h"
#include "vapor/analyzer/variable.h"

namespace reaver
{
    namespace vapor
    {
        namespace analyzer { inline namespace _v1
        {
            class declaration;

            class symbol
            {
            public:
                symbol(std::string name, std::shared_ptr<scope> lex_scope, std::shared_ptr<variable> referenced_variable, std::shared_ptr<declaration> decl)
                    : _name{ std::move(name) }, _lexical_scope{ std::move(lex_scope) }, _variable{ referenced_variable }, _declaration{ std::move(decl) }
                {
                }
            private:
                std::string _name;
                std::shared_ptr<scope> _lexical_scope;
                std::shared_ptr<variable> _variable;
                std::shared_ptr<declaration> _declaration;
            };

            auto make_symbol(std::string name, std::shared_ptr<scope> lex_scope, std::shared_ptr<variable> referenced_variable, std::shared_ptr<declaration> decl)
            {
                return std::make_shared<symbol>(std::move(name), std::move(lex_scope), std::move(referenced_variable), std::move(decl));
            }
        }}
    }
}