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

#include <stack>

#include <boost/filesystem.hpp>

#include "../config/compiler_options.h"
#include "expressions/entity.h"
#include "semantic/context.h"

namespace reaver::vapor::proto
{
class user_defined_reference;
}

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    class unresolved_type;

    struct user_defined_reference_compare
    {
        bool operator()(const proto::user_defined_reference * lhs, const proto::user_defined_reference * rhs) const;
    };

    struct user_defined_reference_hash
    {
        std::size_t operator()(const proto::user_defined_reference * obj) const;
    };

    struct precontext
    {
        const config::compiler_options & options;
        analysis_context & proper;
        std::unordered_map<std::string, std::unique_ptr<entity>> loaded_modules = {};
        std::set<std::unique_ptr<entity>> imported_entities = {};
        std::unordered_map<const proto::user_defined_reference *, std::shared_ptr<unresolved_type>, user_defined_reference_hash, user_defined_reference_compare>
            user_defined_types = {};

        scope * global_scope = nullptr;

        std::stack<boost::filesystem::path> current_file = {};
        std::stack<std::string> current_scope = {};
        std::string current_symbol = {};
    };
}
}
