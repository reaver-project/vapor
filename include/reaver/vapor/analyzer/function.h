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
#include <vector>

#include <reaver/function.h>
#include <reaver/optional.h>

#include "../range.h"
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
            class block;

            using function_codegen = reaver::function<codegen::ir::function (ir_generation_context &)>;
            using function_eval = reaver::function<std::shared_ptr<expression> (optimization_context &, std::vector<std::shared_ptr<variable>>)>;

            class function : public std::enable_shared_from_this<function>
            {
            public:
                function(std::string explanation, std::shared_ptr<type> ret, std::vector<std::shared_ptr<type>> args, function_codegen codegen, optional<range_type> range = none)
                    : _explanation{ std::move(explanation) }, _range{ std::move(range) },
                    _return_type{ std::move(ret) }, _argument_types{ std::move(args) }, _codegen{ std::move(codegen) }
                {
                }

                std::shared_ptr<type> return_type() const
                {
                    return _return_type;
                }

                std::vector<std::shared_ptr<type>> arguments() const
                {
                    return _argument_types;
                }

                std::string explain() const
                {
                    auto ret = _explanation;
                    fmap(_range, [&ret](auto && r) {
                        std::stringstream stream;
                        stream << " (at " << r << ")";
                        ret += stream.str();

                        return unit{};
                    });
                    return ret;
                }

                future<> simplify(optimization_context &);
                future<std::shared_ptr<expression>> simplify(optimization_context &, std::vector<std::shared_ptr<variable>>);

                codegen::ir::function codegen_ir(ir_generation_context & ctx) const
                {
                    auto state = ctx.top_level_generation;
                    ctx.top_level_generation = false;

                    if (!_ir)
                    {
                        _ir = _codegen(ctx);
                    }

                    if (state)
                    {
                        ctx.add_generated_function(shared_from_this());
                    }

                    ctx.top_level_generation = state;

                    return *_ir;
                }

                codegen::ir::value call_operand_ir(ir_generation_context & ctx) const
                {
                    return { codegen::ir::label{ codegen_ir(ctx).name, {} } };
                }

                void set_body(std::weak_ptr<block> body)
                {
                    _body = std::move(body);
                }

                void set_eval(function_eval eval)
                {
                    _compile_time_eval = std::move(eval);
                }

            private:
                std::string _explanation;
                optional<range_type> _range;

                std::weak_ptr<block> _body;
                std::shared_ptr<type> _return_type;
                std::vector<std::shared_ptr<type>> _argument_types;
                function_codegen _codegen;
                mutable optional<codegen::ir::function> _ir;

                optional<function_eval> _compile_time_eval;
            };

            inline std::shared_ptr<function> make_function(std::string expl, std::shared_ptr<type> return_type, std::vector<std::shared_ptr<type>> arguments, function_codegen codegen, optional<range_type> range = none)
            {
                return std::make_shared<function>(std::move(expl), std::move(return_type), std::move(arguments), std::move(codegen), std::move(range));
            }
        }}
    }
}

