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

#include <boost/algorithm/string.hpp>

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
                id_expression(const parser::id_expression & parse, scope * lex_scope) : _parse{ parse }, _lex_scope{ lex_scope }
                {
                }

                std::u32string name() const
                {
                    return boost::join(fmap(_parse.id_expression_value, [](auto && elem) -> decltype(auto) { return elem.string; }), ".");
                }

                virtual void print(std::ostream & os, std::size_t indent) const override;

            private:
                virtual future<> _analyze() override;
                virtual future<expression *> _simplify_expr(optimization_context &) override;
                virtual statement_ir _codegen_ir(ir_generation_context &) const override;

                const parser::id_expression & _parse;
                scope * _lex_scope;
                variable * _referenced;
            };

            inline std::unique_ptr<id_expression> preanalyze_id_expression(const parser::id_expression & parse, scope * lex_scope)
            {
                return std::make_unique<id_expression>(parse, lex_scope);
            }
        }}
    }
}

