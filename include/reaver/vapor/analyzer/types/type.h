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

#include <reaver/future.h>
#include <reaver/variant.h>

#include <google/protobuf/message.h>

#include "../../codegen/ir/type.h"
#include "../../lexer/token.h"
#include "../../print_helpers.h"
#include "../ir_context.h"
#include "../semantic/scope.h"

namespace reaver::vapor::codegen
{
inline namespace _v1
{
    namespace ir
    {
        struct variable_type;
    }
}
}

namespace reaver::vapor::proto
{
struct type;
struct type_reference;
}

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    class function;
    class expression;

    class type
    {
    public:
        type() : _member_scope{ std::make_unique<scope>() }
        {
            _init_expr();
            _init_pack_type();
        }

        type(scope * outer_scope) : _member_scope{ outer_scope->clone_for_class() }
        {
            _init_expr();
            _init_pack_type();
        }

        type(std::unique_ptr<scope> member_scope) : _member_scope{ std::move(member_scope) }
        {
            _init_expr();
            _init_pack_type();
        }

        static constexpr struct dont_init_expr_t
        {
        } dont_init_expr{};

        type(dont_init_expr_t, std::unique_ptr<scope> lex_scope = std::make_unique<scope>())
            : _member_scope{ std::move(lex_scope) }
        {
        }

        void init_expr()
        {
            if (!_self_expression)
            {
                if (!_expression_initialization)
                {
                    _expression_initialization = true;
                    _init_expr();
                    _init_pack_type();
                }
            }
        }

    protected:
        // this is virtually only for `pack_type`
        // don't abuse, please
        static constexpr struct dont_init_pack_t
        {
        } dont_init_pack{};

        type(dont_init_pack_t) : _member_scope{ std::make_unique<scope>() }
        {
            _init_expr();
        }

        type(scope * outer_scope, dont_init_pack_t) : _member_scope{ outer_scope->clone_for_class() }
        {
            _init_expr();
        }

    public:
        virtual ~type();

        virtual future<std::vector<function *>> get_candidates(lexer::token_type) const
        {
            return make_ready_future(std::vector<function *>{});
        }

        virtual future<function *> get_constructor(std::vector<const expression *>) const
        {
            return make_ready_future(static_cast<function *>(nullptr));
        }

        virtual std::string explain() const = 0;
        virtual void print(std::ostream & os, print_context ctx) const = 0;

        scope * get_scope() const
        {
            return _member_scope.get();
        }

        virtual type * get_member_type(const std::u32string &) const
        {
            return nullptr;
        }

        std::shared_ptr<codegen::ir::variable_type> codegen_type(ir_generation_context & ctx) const
        {
            if (!_codegen_t)
            {
                _codegen_t = std::make_shared<codegen::ir::variable_type>();
                _codegen_t.value()->scopes = _member_scope->codegen_ir();
                _codegen_type(ctx);
            }

            return *_codegen_t;
        }

        expression * get_expression() const;

        virtual type * get_pack_type() const
        {
            assert(_pack_type);
            return _pack_type.get();
        }

        virtual bool matches(type * other) const
        {
            return this == other;
        }

        virtual bool matches(const std::vector<type *> & types) const
        {
            return false;
        }

        virtual bool needs_conversion(type * other) const
        {
            return false;
        }

        std::vector<codegen::ir::scope> codegen_scopes(ir_generation_context & ctx) const
        {
            auto base_scopes = _member_scope->codegen_ir();
            base_scopes.push_back(codegen::ir::scope{ _codegen_name(ctx), codegen::ir::scope_type::type });
            return base_scopes;
        }

        const std::u32string & get_name() const
        {
            return _name;
        }

        void set_name(std::u32string name)
        {
            _name = std::move(name);
        }

        virtual std::unique_ptr<proto::type> generate_interface() const = 0;
        virtual std::unique_ptr<proto::type_reference> generate_interface_reference() const = 0;

    private:
        virtual void _codegen_type(ir_generation_context &) const = 0;
        virtual std::u32string _codegen_name(ir_generation_context &) const = 0;

    protected:
        std::unique_ptr<scope> _member_scope;
        // only shared to not require a complete definition of expression to be visible
        // (unique_ptr would require that unless I moved all ctors and dtors out of the header)
        bool _expression_initialization = false;
        std::shared_ptr<expression> _self_expression;
        void _init_expr();
        std::unique_ptr<type> _pack_type;
        void _init_pack_type();

        mutable std::optional<std::shared_ptr<codegen::ir::variable_type>> _codegen_t;

        std::u32string _name;
    };

    class user_defined_type : public type
    {
    public:
        using type::type;

        virtual std::unique_ptr<proto::type> generate_interface() const final;
        virtual std::unique_ptr<proto::type_reference> generate_interface_reference() const final;

    private:
        virtual std::unique_ptr<google::protobuf::Message> _user_defined_interface() const = 0;
    };

    std::unique_ptr<type> make_type_type();
    std::unique_ptr<type> make_integer_type();
    std::unique_ptr<type> make_boolean_type();
    std::unique_ptr<type> make_unconstrained_type();

    inline const auto & builtin_types()
    {
        struct builtin_types_t
        {
            using member_t = std::unique_ptr<class type>;

            member_t type;
            member_t integer;
            member_t boolean;
            member_t unconstrained;
        };

        static auto builtins = [] {
            builtin_types_t builtins;

            builtins.type = make_type_type();
            builtins.integer = make_integer_type();
            builtins.boolean = make_boolean_type();
            builtins.unconstrained = make_unconstrained_type();

            return builtins;
        }();

        builtins.type->init_expr();
        builtins.integer->init_expr();
        builtins.boolean->init_expr();
        builtins.unconstrained->init_expr();

        return builtins;
    }
}
}
