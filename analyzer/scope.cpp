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

#include "vapor/analyzer/scope.h"

void reaver::vapor::analyzer::_v1::scope::close()
{
    {
        _ulock lock{ _lock };

        if (_is_closed)
        {
            return;
        }

        _is_closed = true;

        for (auto && promise : _symbol_promises)
        {
            auto it = _symbols.find(promise.first);
            if (it != _symbols.end())
            {
                promise.second.set(it->second);
                continue;
            }

            promise.second.set(std::make_exception_ptr(
                failed_lookup{ promise.first }
            ));
        }

        _symbol_promises.clear();

        _close_promise->set();
    }

    if (_is_shadowing_boundary && _parent)
    {
        _parent->close();
    }
}

bool reaver::vapor::analyzer::_v1::scope::init(const std::u32string & name, std::shared_ptr<reaver::vapor::analyzer::_v1::symbol> symb)
{
    _ulock lock{ _lock };

    for (auto scope = shared_from_this(); scope; scope = scope->_parent)
    {
        if (scope->_symbols.find(name) != scope->_symbols.end())
        {
            return false;
        }

        if (scope->_is_shadowing_boundary)
        {
            break;
        }
    }

    _symbols.emplace(name, symb);

    auto promise_it = _symbol_promises.find(name);
    if (promise_it != _symbol_promises.end())
    {
        promise_it->second.set(symb);
        _symbol_promises.erase(promise_it);
    }

    return true;
}

reaver::future<std::shared_ptr<reaver::vapor::analyzer::_v1::symbol>> reaver::vapor::analyzer::_v1::scope::get_future(const std::u32string & name)
{
    {
        _shlock lock{ _lock };
        auto it = _symbol_futures.find(name);
        if (it != _symbol_futures.end())
        {
            return it->second;
        }

        auto value_it = _symbols.find(name);
        if (value_it != _symbols.end())
        {
            return _symbol_futures.emplace(name, make_ready_future(value_it->second)).first->second;
        }

        if (_is_closed)
        {
            return make_exceptional_future<std::shared_ptr<symbol>>(failed_lookup{ name });
        }
    }

    _ulock lock{ _lock };

    // need to repeat due to a logical race between the check before
    // and re-locking the lock
    auto it = _symbol_futures.find(name);
    if (it != _symbol_futures.end())
    {
        return it->second;
    }

    auto value_it = _symbols.find(name);
    if (value_it != _symbols.end())
    {
        return _symbol_futures.emplace(name, make_ready_future(value_it->second)).first->second;
    }

    auto pair = make_promise<std::shared_ptr<symbol>>();
    _symbol_promises.emplace(name, std::move(pair.promise));
    return _symbol_futures.emplace(name, std::move(pair.future)).first->second;
}

reaver::future<std::shared_ptr<reaver::vapor::analyzer::_v1::symbol>> reaver::vapor::analyzer::_v1::scope::resolve(const std::u32string & name)
{
    {
        _shlock lock{ _lock };

        auto it = _symbol_futures.find(name);
        if (it != _symbol_futures.end())
        {
            return it->second;
        }
    }

    auto pair = make_promise<std::shared_ptr<symbol>>();

    get_future(name).then([promise = pair.promise](auto && symb) {
        promise.set(symb);
    }).on_error([&name, promise = pair.promise, parent = _parent](auto exptr) {
        try
        {
            std::rethrow_exception(exptr);
        }

        catch (failed_lookup & ex)
        {
            if (!parent)
            {
                promise.set(exptr);
                return;
            }

            parent->resolve(name).then([promise = promise](auto && symb){
                promise.set(symb);
            }).on_error([promise = promise](auto && ex){
                promise.set(ex);
            }).detach();
        }

        catch (...)
        {
            promise.set(exptr);
        }
    }).detach();

    _ulock lock{ _lock };
    _symbol_futures.emplace(name, pair.future);
    return std::move(pair.future);
}

