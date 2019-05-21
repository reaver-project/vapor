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

#include "vapor/codegen/ir/scope.h"
#include <reaver/logger.h>

namespace reaver::vapor::codegen
{
inline namespace _v1
{
    namespace ir
    {
        bool same_module(const std::vector<scope> & lhs, const std::vector<scope> & rhs)
        {
            auto lhs_it = lhs.begin();
            auto rhs_it = rhs.begin();

            while (lhs_it != lhs.end())
            {
                if (rhs_it == rhs.end())
                {
                    return lhs_it->type != scope_type::module;
                }

                if (lhs_it->type == scope_type::module)
                {
                    if (rhs_it->type != scope_type::module)
                    {
                        return false;
                    }

                    if (lhs_it->name != rhs_it->name)
                    {
                        return false;
                    }
                }

                else if (rhs_it->type == scope_type::module)
                {
                    return false;
                }

                ++lhs_it;
                ++rhs_it;
            }

            return rhs_it == rhs.end() || rhs_it->type != scope_type::module;
        }
    }
}
}
