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

#include "type.h"
#include "../codegen/ir/variable.h"
#include "../codegen/ir/function.h"
#include "ir_context.h"
#include "optimization_context.h"

namespace reaver
{
    namespace vapor
    {
        namespace analyzer { inline namespace _v1
        {
            class type;
            class expression;

            using variable_ir = std::vector<variant<codegen::ir::value, codegen::ir::function>>;

            class variable : public std::enable_shared_from_this<variable>
            {
            public:
                virtual ~variable() = default;

                virtual std::shared_ptr<type> get_type() const = 0;

                future<std::shared_ptr<variable>> simplify(optimization_context & ctx)
                {
                    return ctx.get_future_or_init(this, [&]() {
                        return make_ready_future().then([&]{
                            return _simplify(ctx);
                        });
                    });
                }

                variable_ir codegen_ir(ir_generation_context & ctx) const
                {
                    if (!_ir)
                    {
                        _ir = _codegen_ir(ctx);
                    }

                    return *_ir;
                }

                virtual bool is_constant() const
                {
                    return false;
                }

            private:
                virtual future<std::shared_ptr<variable>> _simplify(optimization_context &)
                {
                    return make_ready_future(shared_from_this());
                }

                virtual variable_ir _codegen_ir(ir_generation_context &) const = 0;

                mutable optional<variable_ir> _ir;
            };

            class expression_variable : public variable
            {
            public:
                expression_variable(std::weak_ptr<expression> expr, std::shared_ptr<type> type) : _expression{ expr }, _type{ type }
                {
                }

                virtual std::shared_ptr<type> get_type() const override
                {
                    return _type;
                }

            private:
                virtual future<std::shared_ptr<variable>> _simplify(optimization_context & ctx) override;
                virtual variable_ir _codegen_ir(ir_generation_context &) const override;

                std::weak_ptr<expression> _expression;
                std::shared_ptr<type> _type;
            };

            inline std::shared_ptr<variable> make_expression_variable(std::weak_ptr<expression> expr, std::shared_ptr<type> type)
            {
                return std::make_shared<expression_variable>(expr, type);
            }
        }}
    }
}

