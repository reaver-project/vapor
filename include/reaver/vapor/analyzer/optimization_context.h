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

#include <unordered_map>
#include <shared_mutex>

#include <reaver/future.h>

namespace reaver
{
    namespace vapor
    {
        namespace analyzer { inline namespace _v1
        {
            class statement;
            class expression;
            class variable;

            class optimization_context
            {
            public:
                template<typename T, typename F>
                future<std::shared_ptr<T>> get_future_or_init(T * ptr, F && f)
                {
                    auto && futs = _get_futures<T>();

                    {
                        _shlock lock{ _futures_lock };
                        auto it = futs.find(ptr);
                        if (it != futs.end())
                        {
                            return it->second;
                        }
                    }

                    _ulock lock{ _futures_lock };
                    auto it = futs.find(ptr);
                    if (it != futs.end())
                    {
                        return it->second;
                    }

                    auto fut = std::forward<F>(f)();
                    futs.emplace(ptr, fut);

                    _handle_expressions(ptr, fut);

                    return fut;
                }

                void something_heppened()
                {
                    _something_heppened = true;
                }

                bool did_something_happen() const
                {
                    return _something_heppened;
                }

            private:
                std::atomic<bool> _something_heppened{ false };

                using _ulock = std::unique_lock<std::shared_mutex>;
                using _shlock = std::shared_lock<std::shared_mutex>;

                mutable std::shared_mutex _futures_lock;
                std::unordered_map<statement *, future<std::shared_ptr<statement>>> _statement_futures;
                std::unordered_map<expression *, future<std::shared_ptr<expression>>> _expression_futures;
                std::unordered_map<variable *, future<std::shared_ptr<variable>>> _variable_futures;

                template<typename T>
                auto _get_futures() = delete;

                template<typename T, typename Future>
                void _handle_expressions(T *, Future &&)
                {
                }

                // sorry for this, but need a definition of expression for this one thing...
                void _handle_expressions(expression * ptr, future<std::shared_ptr<expression>> & fut);
            };

            template<>
            inline auto optimization_context::_get_futures<statement>()
            {
                return _statement_futures;
            }

            template<>
            inline auto optimization_context::_get_futures<expression>()
            {
                return _expression_futures;
            }

            template<>
            inline auto optimization_context::_get_futures<variable>()
            {
                return _variable_futures;
            }
        }}
    }
}

