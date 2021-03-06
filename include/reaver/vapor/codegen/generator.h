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

#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "ir/entity.h"

namespace reaver::vapor::codegen
{
inline namespace _v1
{
    namespace ir
    {
        struct type;
        struct variable;
        struct function;
        struct instruction;
        struct member_variable;
    }

    class code_generator;

    class codegen_context
    {
    public:
        codegen_context(std::shared_ptr<code_generator> gen) : _generator{ std::move(gen) }
        {
        }

        std::u32string declare_if_necessary(std::shared_ptr<ir::type>);
        std::u32string define_if_necessary(std::shared_ptr<ir::type>);

        std::size_t unnamed_variable_index = 0;
        std::size_t storage_object_index = 0;
        std::u32string put_into_global_before;
        std::u32string put_into_global;
        std::u32string put_into_function_header;

        std::shared_ptr<ir::type> declaring_members_for;
        bool in_function_definition = false;

        int nested_indent = 0;

        auto & generator() const
        {
            return *_generator;
        }

    private:
        std::unordered_set<std::shared_ptr<ir::type>> _declared_types;
        std::unordered_set<std::shared_ptr<ir::type>> _defined_types;
        std::shared_ptr<code_generator> _generator;
    };

    class code_generator
    {
    public:
        virtual ~code_generator() = default;

        virtual std::u32string generate_global_definitions(codegen_context &) const
        {
            return {};
        }

        virtual std::u32string generate_declarations(std::vector<ir::entity> &, codegen_context &) const
        {
            return {};
        }

        virtual std::u32string generate_declaration(std::shared_ptr<ir::type>, codegen_context &) const
        {
            return {};
        }

        virtual std::u32string generate_definitions(std::vector<ir::entity> &, codegen_context &) = 0;
        virtual std::u32string generate_definition(std::shared_ptr<ir::type> type, codegen_context &) = 0;
    };
}
}
