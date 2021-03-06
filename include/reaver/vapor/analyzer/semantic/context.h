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

#pragma once

#include "../simplification/context.h"
#include "signature.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    class sized_integer;
    class function_type;
    class typeclass_type;

    struct argument_list_hash
    {
        std::size_t operator()(const std::vector<expression *> & arg_list) const;
    };

    struct argument_list_compare
    {
        bool operator()(const std::vector<expression *> & lhs, const std::vector<expression *> & rhs) const;
    };

    struct parameter_type_list_hash
    {
        std::size_t operator()(const std::vector<type *> & param_type_list) const;
    };

    struct parameter_type_list_compare
    {
        bool operator()(const std::vector<type *> & lhs, const std::vector<type *> & rhs) const;
    };

    class analysis_context
    {
    public:
        analysis_context()
            : results{ std::make_shared<cached_results>() },
              simplification_ctx{ std::make_shared<simplification_context>(*results) }
        {
        }

        sized_integer * get_sized_integer_type(std::size_t size);
        function_type * get_function_type(function_signature sig);
        typeclass_type * get_typeclass_type(std::vector<type *> param_types);

        std::shared_ptr<cached_results> results;
        std::shared_ptr<simplification_context> simplification_ctx;

        bool entry_point_marked = false;
        bool entry_variable_marked = false;

    private:
        std::unordered_map<std::size_t, std::shared_ptr<sized_integer>> _sized_integers;
        std::unordered_map<function_signature, std::shared_ptr<function_type>> _function_types;
        std::unordered_map<std::vector<type *>,
            std::shared_ptr<typeclass_type>,
            parameter_type_list_hash,
            parameter_type_list_compare>
            _typeclass_types;
    };
}
}
