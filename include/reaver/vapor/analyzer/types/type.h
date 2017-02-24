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

#include <reaver/variant.h>

#include "../../codegen/ir/type.h"
#include "../../lexer/token.h"
#include "../ir_context.h"
#include "../scope.h"

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

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    class function;
    class expression;
    class variable;

    class type
    {
    public:
        type() : _member_scope{ std::make_unique<scope>() }
        {
        }

        type(scope * outer_scope) : _member_scope{ outer_scope->clone_for_class() }
        {
        }

        virtual ~type();

        virtual future<std::vector<function *>> get_candidates(lexer::token_type) const
        {
            return make_ready_future(std::vector<function *>{});
        }

        virtual future<function *> get_constructor(std::vector<const variable *>) const
        {
            return make_ready_future(static_cast<function *>(nullptr));
        }

        virtual std::string explain() const = 0;

        virtual const scope * get_scope() const
        {
            return _member_scope.get();
        }

        std::shared_ptr<codegen::ir::variable_type> codegen_type(ir_generation_context & ctx) const
        {
            if (!_codegen_t)
            {
                _codegen_t = std::make_shared<codegen::ir::variable_type>();
                _codegen_type(ctx);
            }

            return *_codegen_t;
        }

        expression * get_expression();

    private:
        virtual void _codegen_type(ir_generation_context &) const = 0;

    protected:
        std::unique_ptr<scope> _member_scope;
        // only shared to not require a complete definition of expression to be visible
        // (unique_ptr would require that unless I moved all ctors and dtors out of the header)
        mutable std::shared_ptr<expression> _self_expression;

        mutable optional<std::shared_ptr<codegen::ir::variable_type>> _codegen_t;
    };

    class type_type : public type
    {
    public:
        virtual std::string explain() const override
        {
            return "type";
        }

        virtual future<std::vector<function *>> get_candidates(lexer::token_type token) const override;

    private:
        virtual void _codegen_type(ir_generation_context &) const override;

        mutable std::mutex _generic_ctor_lock;
        mutable std::shared_ptr<function> _generic_ctor;
        mutable std::shared_ptr<variable> _generic_ctor_first_arg;
        mutable std::shared_ptr<variable> _generic_ctor_pack_arg;
    };

    std::unique_ptr<type> make_integer_type();
    std::unique_ptr<type> make_boolean_type();

    inline const auto & builtin_types()
    {
        struct builtin_types_t
        {
            using member_t = std::unique_ptr<class type>;

            member_t type;
            member_t integer;
            member_t boolean;
        };

        static auto builtins = [] {
            builtin_types_t builtins;

            builtins.type = std::make_unique<type_type>();
            builtins.integer = make_integer_type();
            builtins.boolean = make_boolean_type();

            return builtins;
        }();

        return builtins;
    }
}
}
