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
#include <unordered_map>
#include <string>
#include <shared_mutex>

#include <reaver/optional.h>
#include <reaver/future.h>

#include "../utf8.h"

namespace reaver
{
    namespace vapor
    {
        namespace analyzer { inline namespace _v1
        {
            class failed_lookup : public exception
            {
            public:
                failed_lookup(const std::u32string & n) : exception{ logger::error }, name{ n }
                {
                    *this << "failed scope lookup for `" << utf8(name) << "`.";
                }

                std::u32string name;
            };

            class symbol;

            class scope : public std::enable_shared_from_this<scope>
            {
                struct _key {};

                void _init_close()
                {
                    auto pair = make_promise<void>();
                    _close_future = std::move(pair.future);
                    _close_promise = std::move(pair.promise);
                }

            public:
                scope(bool is_local = false) : _is_local_scope{ is_local }
                {
                    _init_close();
                }

            private:
                using _ulock = std::unique_lock<std::shared_mutex>;
                using _shlock = std::shared_lock<std::shared_mutex>;

            public:
                scope(_key, std::shared_ptr<scope> parent_scope, bool is_local, bool is_shadowing_boundary) : _parent{ std::move(parent_scope) }, _is_local_scope{ is_local }, _is_shadowing_boundary{ is_shadowing_boundary }
                {
                    _init_close();
                }

                ~scope() noexcept
                {
                    close();
                }

                void close()
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

                std::shared_ptr<scope> clone_for_decl()
                {
                    if (_is_local_scope)
                    {
                        return std::make_shared<scope>(_key{}, shared_from_this(), true, false);
                    }

                    return shared_from_this();
                }

                std::shared_ptr<scope> clone_local()
                {
                    return std::make_shared<scope>(_key{}, shared_from_this(), true, true);
                }

                std::shared_ptr<scope> clone_for_class()
                {
                    return std::make_shared<scope>(_key{}, shared_from_this(), false, true);
                }

                auto get(const std::u32string & name) const
                {
                    _shlock lock{ _lock };
                    return _symbols.at(name);
                }

                auto try_get(const std::u32string & name) const
                {
                    _shlock lock{ _lock };
                    auto it = _symbols.find(name);
                    return it != _symbols.end() ? make_optional(it->second) : none;
                }

                bool init(const std::u32string & name, std::shared_ptr<symbol> symb)
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

                template<typename F>
                auto get_or_init(const std::u32string & name, F init)
                {
                    {
                        _shlock lock{ _lock };
                        auto it = _symbols.find(name);
                        if (it != _symbols.end())
                        {
                            return it->second;
                        }
                    }

                    _ulock lock{ _lock };

                    // need to repeat due to a logical race between the check before
                    // and re-locking the lock
                    auto it = _symbols.find(name);
                    if (it != _symbols.end())
                    {
                        return it->second;
                    }

                    auto init_v = init();
                    _symbols.emplace(name, init_v);
                    auto promise_it = _symbol_promises.find(name);
                    if (promise_it != _symbol_promises.end())
                    {
                        promise_it->second.set(init_v);
                        _symbol_promises.erase(promise_it);
                    }

                    return init_v;
                }

                // this will always give you a thingy from *current* scope
                // if you want to get from any of the scopes up
                // do use resolve()
                future<std::shared_ptr<symbol>> get_future(const std::u32string & name)
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

                future<std::shared_ptr<symbol>> resolve(const std::u32string & name)
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

            private:
                mutable std::shared_mutex _lock;

                std::shared_ptr<scope> _parent;
                std::unordered_map<std::u32string, std::shared_ptr<symbol>> _symbols;
                std::unordered_map<std::u32string, future<std::shared_ptr<symbol>>> _symbol_futures;
                std::unordered_map<std::u32string, manual_promise<std::shared_ptr<symbol>>> _symbol_promises;
                const bool _is_local_scope = false;
                const bool _is_shadowing_boundary = false;
                bool _is_closed = false;

                optional<future<>> _close_future;
                optional<manual_promise<void>> _close_promise;
            };
        }}
    }
}

