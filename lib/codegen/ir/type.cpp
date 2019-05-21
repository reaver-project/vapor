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

#include "vapor/codegen/ir/type.h"

#include <boost/functional/hash.hpp>

namespace reaver::vapor::codegen
{
inline namespace _v1
{
    namespace ir::detail
    {
        std::shared_ptr<sized_integer_type> builtin_types_t::sized_integer(std::size_t size) const
        {
            static std::unordered_map<std::size_t, std::shared_ptr<sized_integer_type>> types;

            auto & type = types[size];
            if (!type)
            {
                auto sized_type = std::make_shared<sized_integer_type>();
                sized_type->integer_size = size;
                type = sized_type;
            }
            return type;
        }

        struct signature
        {
            std::shared_ptr<type> return_type;
            std::vector<std::shared_ptr<type>> parameter_types;

            bool operator==(const signature & other) const
            {
                return return_type == other.return_type && parameter_types == other.parameter_types;
            }
        };
    }
}
}

namespace std
{
template<>
struct hash<reaver::vapor::codegen::ir::detail::signature>
{
    std::size_t operator()(const reaver::vapor::codegen::ir::detail::signature & sign) const
    {
        std::size_t seed = 0;

        boost::hash_combine(seed, sign.return_type.get());
        boost::hash_combine(seed, sign.parameter_types.size());
        for (auto && param : sign.parameter_types)
        {
            boost::hash_combine(seed, param.get());
        }

        return seed;
    }
};
}

namespace reaver::vapor::codegen
{
inline namespace _v1
{
    namespace ir::detail
    {
        std::shared_ptr<function_type> builtin_types_t::function(std::shared_ptr<struct type> return_type,
            std::vector<std::shared_ptr<struct type>> parameter_types) const
        {
            static std::unordered_map<signature, std::shared_ptr<function_type>> types;

            auto sign = signature{ std::move(return_type), std::move(parameter_types) };

            auto & type = types[sign];
            if (!type)
            {
                auto fn_type = std::make_shared<function_type>();
                fn_type->return_type = std::move(sign.return_type);
                fn_type->parameter_types = std::move(sign.parameter_types);
                type = fn_type;
            }
            return type;
        }
    }
}
}
