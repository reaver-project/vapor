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
#include "simplification_context.h"

namespace reaver
{
    namespace vapor
    {
        namespace analyzer { inline namespace _v1
        {
            class type;
            class block;

            using function_codegen = reaver::function<codegen::ir::function (ir_generation_context &)>;
            using function_eval = reaver::function<expression * (simplification_context &, std::vector<variable *>)>;

            class function
            {
            public:
                function(std::string explanation, type * ret, std::vector<variable *> args, function_codegen codegen, optional<range_type> range = none)
                    : _explanation{ std::move(explanation) }, _range{ std::move(range) },
                    _return_type{ ret }, _arguments{ std::move(args) }, _codegen{ std::move(codegen) }
                {
                    if (ret)
                    {
                        _return_type_future = make_ready_future(ret);
                        return;
                    }

                    auto pair = make_promise<type *>();
                    _return_type_promise = std::move(pair.promise);
                    _return_type_future = std::move(pair.future);
                }

                future<type *> return_type() const
                {
                    return *_return_type_future;
                }

                std::vector<variable *> arguments() const
                {
                    return _arguments;
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

                future<> simplify(simplification_context &);
                future<expression *> simplify(simplification_context &, std::vector<variable *>);

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
                        ctx.add_generated_function(this);
                    }

                    ctx.top_level_generation = state;

                    return *_ir;
                }

                codegen::ir::value call_operand_ir(ir_generation_context & ctx) const
                {
                    assert(_name);
                    return { codegen::ir::label{ *_name, {} } };
                }

                void set_return_type(type * ret)
                {
                    std::unique_lock<std::mutex> lock{ _ret_lock };
                    _return_type = ret;
                    fmap(_return_type_promise, [ret](auto && promise) { promise.set(ret); return unit{}; });
                }

                void set_name(std::u32string name)
                {
                    _name = name;
                }

                void set_body(block * body)
                {
                    _body = body;
                }

                block * get_body() const
                {
                    return _body;
                }

                void set_eval(function_eval eval)
                {
                    _compile_time_eval = std::move(eval);
                }

                void set_arguments(std::vector<variable *> args)
                {
                    _arguments = std::move(args);
                }

            private:
                std::string _explanation;
                optional<range_type> _range;

                block * _body = nullptr;
                type * _return_type;
                mutable std::mutex _ret_lock;
                optional<future<type *>> _return_type_future;
                optional<manual_promise<type *>> _return_type_promise;

                std::vector<variable *> _arguments;
                optional<std::u32string> _name;
                function_codegen _codegen;
                mutable optional<codegen::ir::function> _ir;

                optional<function_eval> _compile_time_eval;
            };

            inline std::unique_ptr<function> make_function(std::string expl, type * return_type, std::vector<variable *> arguments, function_codegen codegen, optional<range_type> range = none)
            {
                return std::make_unique<function>(std::move(expl), std::move(return_type), std::move(arguments), std::move(codegen), std::move(range));
            }
        }}
    }
}

