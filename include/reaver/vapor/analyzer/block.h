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

#include <reaver/prelude/monad.h>

#include "../parser/block.h"
#include "scope.h"
#include "statement.h"
#include "expression.h"
#include "return.h"

namespace reaver
{
    namespace vapor
    {
        namespace analyzer { inline namespace _v1
        {
            class block;
            std::shared_ptr<block> preanalyze_block(const parser::block &, std::shared_ptr<scope>);

            class block : public statement
            {
            public:
                block(const parser::block & parse, std::shared_ptr<scope> lex_scope);

                std::shared_ptr<type> value_type() const
                {
                    if (_value_expr)
                    {
                        return (*_value_expr)->get_type();
                    }

                    return nullptr;
                }

                std::shared_ptr<type> return_type() const
                {
                    auto return_types = fmap(get_returns(), [](auto && stmt){ return stmt->get_returned_type(); });
                    return_types.push_back(value_type());
                    assert(return_types.size() == 1);
                    return return_types.front();
                }

                virtual std::vector<std::shared_ptr<const return_statement>> get_returns() const override
                {
                    return mbind(_statements, [](auto && stmt){ return stmt->get_returns(); });
                }

                virtual void print(std::ostream & os, std::size_t indent) const override;

            private:
                virtual future<> _analyze() override;

                const parser::block & _parse;
                std::shared_ptr<scope> _scope;
                std::vector<std::shared_ptr<statement>> _statements;
                optional<std::shared_ptr<expression>> _value_expr;
            };

            inline std::shared_ptr<block> preanalyze_block(const parser::block & parse, std::shared_ptr<scope> lex_scope)
            {
                return std::make_shared<block>(parse, std::move(lex_scope));
            }
        }}
    }
}

