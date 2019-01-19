/**
 * Vapor Compiler Licence
 *
 * Copyright © 2016-2019 Michał "Griwes" Dominiak
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

#include "vapor/analyzer/semantic/context.h"
#include "vapor/analyzer/expressions/expression.h"
#include "vapor/analyzer/semantic/symbol.h"
#include "vapor/analyzer/statements/statement.h"
#include "vapor/analyzer/types/function.h"
#include "vapor/analyzer/types/sized_integer.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    std::size_t argument_list_hash::operator()(const std::vector<expression *> & arg_list) const
    {
        std::size_t seed = 0;

        boost::hash_combine(seed, arg_list.size());
        for (auto && arg : arg_list)
        {
            boost::hash_combine(seed, arg->hash_value());
        }

        return seed;
    }

    bool argument_list_compare::operator()(const std::vector<expression *> & lhs,
        const std::vector<expression *> & rhs) const
    {
        return lhs.size() == rhs.size()
            && std::equal(lhs.begin(), lhs.end(), rhs.begin(), [](auto && lhs, auto && rhs) {
                   return lhs->is_equal(rhs);
               });
    }

    function_type * analysis_context::get_function_type(function_signature sig)
    {
        auto & ret = _function_types[sig];
        if (!ret)
        {
            ret = std::make_unique<function_type>(sig.return_type, std::move(sig.parameters));
        }

        return ret.get();
    }

    type * analysis_context::get_sized_integer_type(std::size_t size)
    {
        auto & ret = _sized_integers[size];
        if (!ret)
        {
            ret = make_sized_integer_type(size);
        }

        return ret.get();
    }
}
}
