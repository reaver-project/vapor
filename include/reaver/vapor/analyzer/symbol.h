/**
 * Vapor Compiler Licence
 *
 * Copyright © 2014, 2016 Michał "Griwes" Dominiak
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

#include "variable.h"

namespace reaver
{
    namespace vapor
    {
        namespace analyzer { inline namespace _v1
        {
            class symbol
            {
                using _ulock = std::unique_lock<std::shared_mutex>;
                using _shlock = std::shared_lock<std::shared_mutex>;

            public:
                symbol(std::u32string name, std::shared_ptr<variable> variable)
                    : _name{ std::move(name) }, _variable{ std::move(variable) }
                {
                }

                void set_variable(std::shared_ptr<variable> var)
                {
                    _ulock lock{ _lock };

                    assert(!_variable);
                    _variable = std::move(var);
                    if (_promise)
                    {
                        _promise->set(_variable);
                    }
                }

                std::shared_ptr<variable> get_variable() const
                {
                    _shlock lock{ _lock };

                    assert(_variable);
                    return _variable;
                }

                std::shared_ptr<type> get_type() const
                {
                    _shlock lock{ _lock };

                    assert(_variable);
                    return _variable->get_type();
                }

                auto get_variable_future()
                {
                    _ulock lock{ _lock };

                    if (_variable)
                    {
                        if (!_future)
                        {
                            _future = make_ready_future(_variable);
                        }
                        return *_future;
                    }

                    auto pair = make_promise<std::shared_ptr<variable>>();
                    _promise = std::move(pair.promise);
                    _future = std::move(pair.future);
                    return *_future;
                }

                std::vector<variant<std::shared_ptr<codegen::ir::variable>, codegen::ir::function>> codegen_ir() const;

            private:
                mutable std::shared_mutex _lock;

                std::u32string _name;

                std::shared_ptr<variable> _variable;
                optional<future<std::shared_ptr<variable>>> _future;
                optional<manual_promise<std::shared_ptr<variable>>> _promise;
            };

            inline auto make_symbol(std::u32string name, std::shared_ptr<variable> variable = nullptr)
            {
                return std::make_shared<symbol>(std::move(name), std::move(variable));
            }
        }}
    }
}

