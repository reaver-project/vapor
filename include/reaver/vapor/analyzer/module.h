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

#include <vector>
#include <string>
#include <map>

#include <boost/algorithm/string.hpp>

#include "vapor/range.h"
#include "vapor/parser/module.h"
#include "vapor/parser/id_expression.h"
#include "vapor/analyzer/declaration.h"
#include "vapor/analyzer/scope.h"
#include "vapor/analyzer/import.h"
#include "vapor/analyzer/symbol.h"
#include "vapor/analyzer/helpers.h"
#include "vapor/analyzer/statement.h"

namespace reaver
{
    namespace vapor
    {
        namespace analyzer { inline namespace _v1
        {
            class module : public scope, public std::enable_shared_from_this<module>
            {
            public:
                module(const parser::module & parse) : _parse{ parse }
                {
                }

                void analyze()
                {
                    for (auto && statement : _parse.statements)
                    {
                        visit(make_visitor(
                            shptr_id<declaration>(), [&](const std::shared_ptr<declaration> & decl)
                            {
                                auto & symb = _symbols[decl->name()];
                                if (symb)
                                {
                                    error("redefinition of `" + utf8(decl->name()) + "`", decl->parse());
                                }
                                else
                                {
                                    symb = decl->declared_symbol();
                                }

                                return unit{};
                            },

                            shptr_id<import>(), [&](const std::shared_ptr<import> & im)
                            {
                                assert(0);
                                return unit{};
                            },

                            shptr_id<expression>(), [&](auto &&...)
                            {
                                throw exception{ logger::crash } << "got invalid statement in module; fix the parser";
                                return unit{};
                            }
                        ), preanalyze_statement(statement, shared_from_this()));
                    }
                }

                std::u32string name() const
                {
                    std::vector<std::u32string> tmp;
                    tmp.reserve(_parse.name.id_expression_value.size());
                    std::transform(_parse.name.id_expression_value.begin(), _parse.name.id_expression_value.end(), std::back_inserter(tmp), [](auto && elem) -> decltype(auto) { return elem.string; });
                    return boost::join(tmp, ".");
                }

            private:
                const parser::module & _parse;
                std::unordered_map<std::u32string, std::shared_ptr<symbol>> _symbols;
            };
        }}
    }
}
