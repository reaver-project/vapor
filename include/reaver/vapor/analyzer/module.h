/**
 * Vapor Compiler Licence
 *
 * Copyright © 2014, 2016 Michał "Griwes" Dominiak
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

#include "../range.h"
#include "../parser/module.h"
#include "../parser/id_expression.h"
#include "declaration.h"
#include "scope.h"
#include "import.h"
#include "symbol.h"
#include "helpers.h"
#include "statement.h"
#include "function.h"

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
                    fmap(
                        fmap(
                            _parse.statements,
                            [&](const auto & statement){
                                return preanalyze_statement(statement, enable_shared_from_this<module>::shared_from_this());
                            }
                        ),
                        make_overload_set(
                            [&](const std::shared_ptr<declaration> & decl)
                            {
                                auto & symb = get_ref(decl->name());
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

                            [&](const std::shared_ptr<import> & im)
                            {
                                assert(0);
                                return unit{};
                            },

                            [&](const std::shared_ptr<function> & fun)
                            {
                                assert(0);
                                return unit{};
                            },

                            [&](auto &&...)
                            {
                                throw exception{ logger::crash } << "got invalid statement in module; fix the parser";
                                return unit{};
                            }
                        )
                    );
                }

                std::u32string name() const
                {
                    return boost::join(fmap(_parse.name.id_expression_value, [](auto && elem) -> decltype(auto) { return elem.string; }), ".");
                }

            private:
                const parser::module & _parse;
            };
        }}
    }
}
