/**
 * Vapor Compiler Licence
 *
 * Copyright © 2016-2017 Michał "Griwes" Dominiak
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

namespace reaver::vapor::codegen
{
inline namespace _v1
{
    namespace ir
    {
        struct variable_type;
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

        std::u32string declare_if_necessary(std::shared_ptr<ir::variable_type>);
        std::u32string define_if_necessary(std::shared_ptr<ir::variable_type>);

        std::size_t unnamed_variable_index = 0;
        std::size_t storage_object_index = 0;
        std::u32string put_into_global_before;
        std::u32string put_into_global;
        std::u32string put_into_function_header;

        std::shared_ptr<ir::variable_type> declaring_members_for;

        auto & generator() const
        {
            return *_generator;
        }

    private:
        std::unordered_set<std::shared_ptr<ir::variable_type>> _declared_types;
        std::unordered_set<std::shared_ptr<ir::variable_type>> _defined_types;
        std::shared_ptr<code_generator> _generator;
    };

    class code_generator
    {
    public:
        virtual ~code_generator() = default;

        virtual std::u32string generate_declaration(ir::variable &, codegen_context &) const = 0;
        virtual std::u32string generate_declaration(ir::function &, codegen_context &) const = 0;
        virtual std::u32string generate_declaration(const std::shared_ptr<ir::variable_type> &, codegen_context &) const = 0;
        virtual std::u32string generate_definition(const ir::variable &, codegen_context &) = 0;
        virtual std::u32string generate_definition(const ir::function &, codegen_context &) = 0;
        virtual std::u32string generate_definition(const std::shared_ptr<ir::variable_type> &, codegen_context &) = 0;
        virtual std::u32string generate_definition(const ir::member_variable &, codegen_context &) = 0;

        virtual std::u32string generate(const ir::instruction &, codegen_context &) = 0;
    };
}
}
