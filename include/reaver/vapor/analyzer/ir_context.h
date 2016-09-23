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

#include <vector>
#include <memory>
#include <unordered_set>

#include <reaver/logger.h>

namespace reaver
{
    namespace vapor
    {
        namespace analyzer { inline namespace _v1
        {
            class function;

            class ir_generation_context
            {
            public:
                void add_function_to_generate(const std::shared_ptr<const function> & fn);
                void add_generated_function(const std::shared_ptr<const function> & fn);
                std::shared_ptr<const function> function_to_generate();

                bool top_level_generation = true;
                std::size_t overload_set_index = 0;
                std::size_t closure_index = 0;

            private:
                std::vector<std::shared_ptr<const function>> _functions_to_generate;
                std::unordered_set<std::shared_ptr<const function>> _generated_functions;
            };
        }}
    }
}

