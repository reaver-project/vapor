/**
 * Vapor Compiler Licence
 *
 * Copyright © 2017-2019 Michał "Griwes" Dominiak
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

#include "../semantic/instance_context.h"
#include "type.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    class function_declaration;
    class parameter;
    class overload_set;

    // this is a little awkward, but a typeclass *is* a type for its instances...
    class typeclass : public user_defined_type
    {
    public:
        typeclass(ast_node parse,
            std::unique_ptr<scope> member_scope,
            std::vector<std::unique_ptr<parameter>> parameters,
            std::vector<std::unique_ptr<function_declaration>> member_function_decls);

        virtual std::string explain() const override
        {
            return "a typeclass (TODO: add name tracking to this stuff)";
        }

        std::vector<parameter *> get_parameters() const;

        std::vector<function_declaration *> get_member_function_decls() const
        {
            return fmap(_member_function_declarations, [](auto && ptr) { return ptr.get(); });
        }

        auto get_ast_info() const
        {
            return std::make_optional(_parse);
        }

        virtual void print(std::ostream & os, print_context ctx) const override;
        virtual future<std::vector<function *>> get_candidates(lexer::token_type bracket) const override;

    private:
        virtual std::unique_ptr<google::protobuf::Message> _user_defined_interface() const override;

        virtual void _codegen_type(ir_generation_context &) const override
        {
            assert(0);
        }

        virtual std::u32string _codegen_name(ir_generation_context &) const override
        {
            assert(0);
        }

        ast_node _parse;
        bool _is_exported = false;

        std::vector<std::unique_ptr<parameter>> _parameters;

        std::vector<std::unique_ptr<function_declaration>> _member_function_declarations;
        std::vector<function *> _member_functions;
    };

    class typeclass_instance_type : public user_defined_type, public std::enable_shared_from_this<typeclass_instance_type>
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

    private:
        struct _function_instance
        {
            std::unique_ptr<function> instance;
            std::shared_ptr<expression> return_type_expression;
            std::vector<std::unique_ptr<expression>> parameter_expressions;
            std::shared_ptr<class overload_set> overload_set;
        };

        std::vector<expression *> _arguments;
        instance_context _ctx;

        std::unordered_set<std::u32string> _oset_names;
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

    class function_definition;

    class typeclass_instance : public user_defined_type, public std::enable_shared_from_this<typeclass_instance>
    {
    public:
        typeclass_instance(typeclass_instance_type * type, std::unique_ptr<scope> lex_scope, std::vector<std::shared_ptr<overload_set>> osets);

        virtual std::string explain() const override
        {
            return "a typeclass instance (TODO: add tracking)";
        }

        virtual void print(std::ostream & os, print_context ctx) const override;

        function_definition_handler get_function_definition_handler();

        std::vector<function_definition *> get_member_function_defs() const
        {
            return fmap(_member_function_definitions, [](auto && ptr) { return ptr.get(); });
        }

        typeclass_instance_type * get_instance_type() const
        {
            return _instance_type;
        }

        void import_default_definitions();

    private:
        typeclass_instance_type * _instance_type;

        std::vector<std::shared_ptr<overload_set>> _osets;
        std::vector<std::unique_ptr<function_definition>> _member_function_definitions;

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

    std::shared_ptr<typeclass_instance> make_typeclass_instance(typeclass_instance_type * type);

    std::unique_ptr<type> make_typeclass_type();
}
}

namespace reaver::vapor::parser
{
inline namespace _v1
{
    class typeclass_literal;
}
}

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    struct precontext;

    std::unique_ptr<typeclass> make_typeclass(precontext & ctx, const parser::typeclass_literal & parse, scope * lex_scope);
}
}
