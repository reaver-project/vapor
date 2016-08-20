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
            class module
            {
            public:
                module(const parser::module & parse) : _parse{ parse }, _scope{ std::make_shared<scope>() }
                {
                    _statements = fmap(
                        _parse.statements,
                        [&](const auto & statement){
                            return preanalyze_statement(statement, _scope);
                        }
                    );
                }

                void analyze()
                {
                    _analysis_futures = fmap(_statements, [&](auto && stmt) {
                        return stmt->analyze();
                    });

                    auto all = when_all(_analysis_futures);

                    while (!all.try_get())
                    {
                        std::this_thread::sleep_for(std::chrono::nanoseconds(10000));
                    }
                }

                std::u32string name() const
                {
                    return boost::join(fmap(_parse.name.id_expression_value, [](auto && elem) -> decltype(auto) { return elem.string; }), ".");
                }

            private:
                const parser::module & _parse;
                std::shared_ptr<scope> _scope;
                std::vector<std::shared_ptr<statement>> _statements;
                std::vector<future<>> _analysis_futures;
            };
        }}
    }
}

