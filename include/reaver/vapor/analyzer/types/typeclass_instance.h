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

#pragma once

#include "../expressions/overload_set.h"
#include "../semantic/instance_context.h"
#include "type.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    class typeclass;
    class block;

    class typeclass_instance_type : public user_defined_type,
                                    public std::enable_shared_from_this<typeclass_instance_type>
    {
    public:
        typeclass_instance_type(typeclass * tc, std::vector<expression *> arguments);

        virtual std::string explain() const override
        {
            return "a typeclass instance type (TODO: add name tracking to this)";
        }

        virtual void print(std::ostream & os, print_context ctx) const override;

        auto & overload_set_names() const
        {
            return _oset_names;
        }

        auto get_declaration_of(function * fn) const
        {
            return _function_instance_to_template.at(fn);
        }

    private:
        struct _function_instance
        {
            std::unique_ptr<function> instance;
            std::unique_ptr<block> function_body;
            std::shared_ptr<expression> return_type_expression;
            std::vector<std::unique_ptr<expression>> parameter_expressions;
            std::shared_ptr<class overload_set> overload_set;
        };

        std::vector<expression *> _arguments;
        instance_context _ctx;

        std::set<std::u32string> _oset_names;
        std::vector<_function_instance> _function_instances;
        std::unordered_map<function *, function_declaration *> _function_instance_to_template;

        virtual std::unique_ptr<google::protobuf::Message> _user_defined_interface() const override;

        virtual void _codegen_type(ir_generation_context &) const override
        {
            assert(0);
        }

        virtual std::u32string _codegen_name(ir_generation_context &) const override
        {
            assert(0);
        }
    };
}
}

