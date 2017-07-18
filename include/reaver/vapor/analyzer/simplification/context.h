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

#include <shared_mutex>
#include <unordered_map>

#include <reaver/future.h>

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    class statement;
    class expression;

    class simplification_context
    {
    public:
        ~simplification_context();

        template<typename T, typename F>
        future<T *> get_future_or_init(T * ptr, F && f)
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

        void something_happened()
        {
            _something_happened = true;
        }

        bool did_something_happen() const
        {
            return _something_happened;
        }

        void keep_alive(statement * ptr);

    private:
        std::atomic<bool> _something_happened{ false };

        using _ulock = std::unique_lock<std::shared_mutex>;
        using _shlock = std::shared_lock<std::shared_mutex>;

        mutable std::shared_mutex _futures_lock;
        std::unordered_map<statement *, future<statement *>> _statement_futures;
        std::unordered_map<expression *, future<expression *>> _expression_futures;

        std::mutex _keep_alive_lock;
        std::unordered_set<std::unique_ptr<statement>> _keep_alive_stmt;

        template<typename T>
        auto & _get_futures() = delete;

        template<typename T, typename Future>
        void _handle_expressions(T *, Future &&)
        {
        }

        // sorry for this, but need a definition of expression for this one thing...
        void _handle_expressions(expression * ptr, future<expression *> & fut);
    };

    template<>
    inline auto & simplification_context::_get_futures<statement>()
    {
        return _statement_futures;
    }

    template<>
    inline auto & simplification_context::_get_futures<expression>()
    {
        return _expression_futures;
    }
}
}
