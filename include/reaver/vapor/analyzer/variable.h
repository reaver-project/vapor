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

#include <memory>

#include <reaver/optional.h>

#include "type.h"
#include "../codegen/ir/variable.h"
#include "../codegen/ir/function.h"
#include "statement.h"
#include "ir_context.h"
#include "simplification_context.h"

namespace reaver::vapor::analyzer { inline namespace _v1
{
    class type;
    class expression;

    using variable_ir = variant<none_t, codegen::ir::value, std::vector<codegen::ir::function>>;

    inline auto get_ir_variable(const variable_ir & ir)
    {
        return get<std::shared_ptr<codegen::ir::variable>>(
            get<codegen::ir::value>(ir)
        );
    }

    class variable
    {
    public:
        virtual ~variable() = default;

        virtual type * get_type() const = 0;

        std::unique_ptr<variable> clone_with_replacement(replacements & repl) const
        {
            auto ret = _clone_with_replacement(repl);
            repl.variables[this] = ret.get();
            return ret;
        }

        future<variable *> simplify(simplification_context & ctx)
        {
            return ctx.get_future_or_init(this, [&]() {
                return make_ready_future().then([&]() {
                    return _simplify(ctx);
                });
            });
        }

        variable_ir codegen_ir(ir_generation_context & ctx) const
        {
            if (!_ir)
            {
                auto ir = _codegen_ir(ctx);
                // this if guards from inconsistencies arising from reentry into this function
                // this way of solving this can sometimes cause additional work to be done
                // TODO: figure out how to stop that from happening
                if (!_ir)
                {
                    _ir = std::move(ir);
                }
            }

            return *_ir;
        }

        virtual bool is_constant() const
        {
            return false;
        }

        virtual bool is_equal(const variable *) const
        {
            return false;
        }

        bool is_local() const
        {
            return _is_local;
        }

        void mark_local()
        {
            _is_local = true;
        }

    private:
        virtual std::unique_ptr<variable> _clone_with_replacement(replacements &) const = 0;

        virtual future<variable *> _simplify(simplification_context &)
        {
            return make_ready_future(this);
        }

        virtual variable_ir _codegen_ir(ir_generation_context &) const = 0;

        mutable optional<variable_ir> _ir;
        bool _is_local = false;
    };

    class expression_variable : public variable
    {
    public:
        expression_variable(expression * expr, type * type) : _expression{ expr }, _type{ type }
        {
        }

        virtual type * get_type() const override
        {
            return _type;
        }

        virtual bool is_constant() const override;
        virtual bool is_equal(const variable *) const override;

    private:
        virtual std::unique_ptr<variable> _clone_with_replacement(replacements &) const override;
        virtual future<variable *> _simplify(simplification_context & ctx) override;
        virtual variable_ir _codegen_ir(ir_generation_context &) const override;

        expression * _expression;
        type * _type;
    };

    inline std::unique_ptr<variable> make_expression_variable(expression * expr, type * type)
    {
        return std::make_unique<expression_variable>(expr, type);
    }

    class blank_variable : public variable
    {
    public:
        blank_variable(type * t) : _type{ t }
        {
        }

        virtual type * get_type() const override
        {
            return _type;
        }

    private:
        virtual std::unique_ptr<variable> _clone_with_replacement(replacements &) const override
        {
            return std::make_unique<blank_variable>(_type);
        }

        virtual variable_ir _codegen_ir(ir_generation_context & ctx) const override
        {
            return codegen::ir::make_variable(
                _type->codegen_type(ctx)
            );
        }

        type * _type;
    };

    inline std::unique_ptr<variable> make_blank_variable(type * type)
    {
        return std::make_unique<blank_variable>(type);
    }
}}

