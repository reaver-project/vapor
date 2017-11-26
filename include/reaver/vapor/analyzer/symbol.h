/**
 * Vapor Compiler Licence
 *
 * Copyright © 2014, 2016-2017 Michał "Griwes" Dominiak
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
#include <shared_mutex>

#include "expressions/expression.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    class symbol
    {
        using _ulock = std::unique_lock<std::shared_mutex>;
        using _shlock = std::shared_lock<std::shared_mutex>;

    public:
        symbol(std::u32string name, expression * expression) : _name{ std::move(name) }, _expression{ expression }
        {
        }

        void set_expression(expression * var)
        {
            _ulock lock{ _lock };

            assert(!_expression && var);
            _expression = var;
            if (_promise)
            {
                _promise->set(_expression);
            }
        }

        expression * get_expression() const
        {
            _shlock lock{ _lock };

            assert(_expression);
            return _expression;
        }

        type * get_type() const
        {
            _shlock lock{ _lock };

            assert(_expression);
            return _expression->get_type();
        }

        std::u32string get_name() const
        {
            return _name;
        }

        auto get_expression_future()
        {
            _ulock lock{ _lock };

            if (_expression && !_future)
            {
                _future = make_ready_future(_expression);
            }

            if (!_future)
            {
                auto pair = make_promise<expression *>();
                _promise = std::move(pair.promise);
                _future = std::move(pair.future);
            }

            return *_future;
        }

        future<> simplify(recursive_context ctx)
        {
            return get_expression()->simplify_expr(ctx).then([&](auto && simplified) {
                _ulock lock{ _lock };
                _expression = simplified;
            });
        }

        declaration_ir codegen_ir(ir_generation_context &) const;

    private:
        mutable std::shared_mutex _lock;

        std::u32string _name;

        expression * _expression;
        std::optional<future<expression *>> _future;
        std::optional<manual_promise<expression *>> _promise;
    };

    inline auto make_symbol(std::u32string name, expression * expression = nullptr)
    {
        return std::make_unique<symbol>(std::move(name), expression);
    }
}
}
