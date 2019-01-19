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

#include "vapor/analyzer/types/typeclass.h"
#include "vapor/analyzer/expressions/instance.h"
#include "vapor/analyzer/expressions/overload_set.h"
#include "vapor/analyzer/semantic/symbol.h"
#include "vapor/analyzer/statements/function.h"
#include "vapor/parser/expr.h"
#include "vapor/parser/typeclass.h"

#include "expressions/type.pb.h"
#include "type_reference.pb.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    std::unique_ptr<typeclass> make_typeclass(precontext & ctx,
        const parser::typeclass_literal & parse,
        scope * lex_scope)
    {
        auto scope = lex_scope->clone_for_class();
        auto scope_ptr = scope.get();

        auto params = preanalyze_parameter_list(ctx, parse.parameters, scope_ptr);

        std::vector<std::unique_ptr<function_declaration>> fn_decls;

        fmap(parse.members, [&](auto && member) {
            fmap(member,
                make_overload_set(
                    [&](const parser::function_declaration & decl) {
                        fn_decls.push_back(preanalyze_function_declaration(ctx, decl, scope_ptr));
                        return unit{};
                    },
                    [&](const parser::function_definition & def) {
                        fn_decls.push_back(preanalyze_function_definition(ctx, def, scope_ptr));
                        return unit{};
                    }));

            return unit{};
        });

        scope_ptr->close();

        return std::make_unique<typeclass>(
            make_node(parse), std::move(scope), std::move(params), std::move(fn_decls));
    }

    typeclass::typeclass(ast_node parse,
        std::unique_ptr<scope> member_scope,
        std::vector<std::unique_ptr<parameter>> parameters,
        std::vector<std::unique_ptr<function_declaration>> member_function_decls)
        : user_defined_type{ std::move(member_scope) },
          _parse{ parse },
          _parameters{ std::move(parameters) },
          _member_function_declarations{ std::move(member_function_decls) }
    {
    }

    void typeclass::print(std::ostream & os, print_context ctx) const
    {
        os << styles::def << ctx << styles::type << "typeclass";
        print_address_range(os, this);
        os << '\n';
    }

    std::vector<parameter *> typeclass::get_parameters() const
    {
        return fmap(_parameters, [](auto && param) { return param.get(); });
    }

    future<std::vector<function *>> typeclass::get_candidates(lexer::token_type) const
    {
        assert(0);
    }

    std::unique_ptr<google::protobuf::Message> typeclass::_user_defined_interface() const
    {
        assert(0);
    }

    class typeclass_type : public type
    {
    public:
        typeclass_type() : type{ dont_init_expr }
        {
        }

        virtual std::string explain() const override
        {
            return "typeclass";
        }

        virtual void print(std::ostream & os, print_context ctx) const override
        {
            os << styles::def << ctx << styles::type << "typeclass" << styles::def << " @ " << styles::address
               << this << styles::def << ": builtin type\n";
        }

        virtual std::unique_ptr<proto::type> generate_interface() const override
        {
            auto ret = std::make_unique<proto::type>();
            ret->set_allocated_reference(generate_interface_reference().release());
            return ret;
        }

        virtual std::unique_ptr<proto::type_reference> generate_interface_reference() const override
        {
            auto ret = std::make_unique<proto::type_reference>();
            ret->set_builtin(proto::typeclass);
            return ret;
        }

    private:
        virtual void _codegen_type(ir_generation_context &) const override
        {
            assert(0);
        }

        virtual std::u32string _codegen_name(ir_generation_context & ctx) const override
        {
            return U"typeclass";
        }
    };

    std::unique_ptr<type> make_typeclass_type()
    {
        return std::make_unique<typeclass_type>();
    }

    typeclass_instance_type::typeclass_instance_type(typeclass * tc, std::vector<expression *> arguments)
        : user_defined_type{ dont_init_expr },
          _arguments{ std::move(arguments) },
          _ctx{ tc->get_scope(), _arguments }
    {
        _self_expression = std::make_unique<type_expression>(this, type_kind::typeclass);

        auto repl = _ctx.get_replacements();

        for (auto && fn_decl : tc->get_member_function_decls())
        {
            auto fn = fn_decl->get_function();
            auto & name = fn_decl->get_name();

            _function_instance fn_instance;
            fn_instance.instance = make_function(fn->explain(), fn->get_range());
            fn_instance.return_type_expression = repl.copy_claim(fn->return_type_expression());
            fn_instance.instance->set_return_type(fn_instance.return_type_expression);
            fn_instance.parameter_expressions =
                fmap(fn->parameters(), [&](auto && param) { return repl.copy_claim(param); });
            fn_instance.instance->set_parameters(
                fmap(fn_instance.parameter_expressions, [](auto && expr) { return expr.get(); }));

            fn_instance.overload_set = get_overload_set(get_scope(), name);
            fn_instance.overload_set->get_overload_set_type()->add_function(fn_instance.instance.get());

            _oset_names.insert(name);

            _function_instance_to_template.emplace(fn_instance.instance.get(), fn_decl);
            _function_instances.push_back(std::move(fn_instance));
        }
    }

    void typeclass_instance_type::print(std::ostream & os, print_context ctx) const
    {
        assert(0);
    }

    std::unique_ptr<google::protobuf::Message> typeclass_instance_type::_user_defined_interface() const
    {
        assert(0);
    }

    std::shared_ptr<typeclass_instance> make_typeclass_instance(typeclass_instance_type * type)
    {
        auto scope = type->get_scope()->clone_for_class();

        std::vector<std::shared_ptr<overload_set>> osets;
        for (auto && oset_name : type->overload_set_names())
        {
            osets.push_back(create_overload_set(scope.get(), oset_name));
        }
        // close here, because if the preanalysis below *adds* new members, then we have a bug... the
        // assertion that checks for closeness of the scope needs to be somehow weakened here, to allow for
        // more sensible error reporting than `assert`
        scope->close();

        return std::make_shared<typeclass_instance>(type, std::move(scope), std::move(osets));
    }

    typeclass_instance::typeclass_instance(typeclass_instance_type * type,
        std::unique_ptr<scope> lex_scope,
        std::vector<std::shared_ptr<overload_set>> osets)
        : user_defined_type{ dont_init_expr, std::move(lex_scope) },
          _instance_type{ type },
          _osets{ std::move(osets) }
    {
        _self_expression = make_typeclass_instance_expression(this);
        _init_pack_type();
    }

    function_definition_handler typeclass_instance::get_function_definition_handler()
    {
        return [&](precontext & ctx, const parser::function_definition & parse) {
            auto scope = get_scope();
            auto func = preanalyze_function_definition(ctx, parse, scope, true);
            assert(scope == get_scope());
            _member_function_definitions.push_back(std::move(func));
        };
    }

    void typeclass_instance::import_default_definitions()
    {
        assert(0);
    }

    void typeclass_instance::print(std::ostream & os, print_context ctx) const
    {
        assert(0);
    }

    std::unique_ptr<google::protobuf::Message> typeclass_instance::_user_defined_interface() const
    {
        assert(0);
    }
}
}
