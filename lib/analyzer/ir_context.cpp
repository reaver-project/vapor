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

#include "vapor/analyzer/ir_context.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    void ir_generation_context::add_function_to_generate(const function * fn)
    {
        if (_generated_functions.find(fn) != _generated_functions.end())
        {
            return;
        }

        if (std::find(_functions_to_generate.begin(), _functions_to_generate.end(), fn)
            == _functions_to_generate.end())
        {
            _functions_to_generate.push_back(fn);
        }
    }

    void ir_generation_context::add_generated_function(const function * fn)
    {
        _generated_functions.insert(fn);

        auto known_it = std::find(_functions_to_generate.begin(), _functions_to_generate.end(), fn);
        if (known_it != _functions_to_generate.end())
        {
            _functions_to_generate.erase(known_it);
        }
    }

    const function * ir_generation_context::function_to_generate()
    {
        if (_functions_to_generate.size())
        {
            return _functions_to_generate.back();
        }

        return nullptr;
    }
}
}
