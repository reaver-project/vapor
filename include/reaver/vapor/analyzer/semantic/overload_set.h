/**
 * Vapor Compiler Licence
 *
 * Copyright © 2019 Michał "Griwes" Dominiak
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

#include "../types/overload_set.h"
#include "scope.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    class overload_set_expression;
    class function;
    class function_declaration;
    struct imported_function;

    class overload_set
    {
    public:
        overload_set(scope *);

        void add_function(function * fn);
        void add_function(std::unique_ptr<imported_function> fn);
        void add_function(function_declaration * decl);

        const std::vector<function *> & get_overloads() const
        {
            return _functions;
        }

        overload_set_type * get_type() const
        {
            return _type.get();
        }

    private:
        std::unique_ptr<overload_set_type> _type;

        std::vector<function *> _functions;
        std::vector<function_declaration *> _function_decls;
        // shared so that this is destructible without knowing the definition
        std::vector<std::shared_ptr<imported_function>> _imported_functions;
    };
}
}

namespace reaver::vapor::proto
{
class overload_set_type;
class overload_set;
}

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    class precontext;

    std::unique_ptr<overload_set> import_overload_set(precontext &, const proto::overload_set_type &);
}
}
