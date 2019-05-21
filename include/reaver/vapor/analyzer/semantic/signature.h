/**
 * Vapor Compiler Licence
 *
 * Copyright © 2018 Michał "Griwes" Dominiak
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

#include <functional>
#include <vector>

#include <boost/functional/hash.hpp>

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    class type;

    struct function_signature
    {
        std::vector<type *> parameters;
        type * return_type;
    };

    inline bool operator==(const function_signature & lhs, const function_signature & rhs)
    {
        return lhs.parameters == rhs.parameters && lhs.return_type == rhs.return_type;
    }
}
}

template<>
struct std::hash<reaver::vapor::analyzer::_v1::function_signature>
{
    std::size_t operator()(const reaver::vapor::analyzer::function_signature & sig) const
    {
        std::size_t seed = 0;

        boost::hash_combine(seed, sig.return_type);
        boost::hash_combine(seed, sig.parameters.size());
        for (auto && param_type : sig.parameters)
        {
            boost::hash_combine(seed, param_type);
        }

        return seed;
    }
};
