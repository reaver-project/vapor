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
#include "../codegen/ir/scope.h"
#include "ir_context.h"

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

            class scope
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
                scope(_key, scope * parent_scope, bool is_local, bool is_shadowing_boundary) : _parent{ parent_scope }, _is_local_scope{ is_local }, _is_shadowing_boundary{ is_shadowing_boundary }
                {
                    _init_close();
                }

                ~scope();

                void close();

                scope * parent() const
                {
                    return _parent;
                }

                scope * clone_for_decl()
                {
                    if (_is_local_scope)
                    {
                        return new scope{ _key{}, this, _is_local_scope, false };
                    }

                    return this;
                }

                std::unique_ptr<scope> clone_local()
                {
                    return std::make_unique<scope>(_key{}, this, true, true);
                }

                std::unique_ptr<scope> clone_for_class()
                {
                    return std::make_unique<scope>(_key{}, this, false, true);
                }

                auto get(const std::u32string & name) const
                {
                    _shlock lock{ _lock };
                    return _symbols.at(name).get();
                }

                auto try_get(const std::u32string & name) const
                {
                    _shlock lock{ _lock };
                    auto it = _symbols.find(name);
                    return it != _symbols.end() ? make_optional(it->second) : none;
                }

                bool init(const std::u32string & name, std::unique_ptr<symbol> symb);

                template<typename F>
                auto get_or_init(const std::u32string & name, F init)
                {
                    assert(!_is_closed);

                    {
                        _shlock lock{ _lock };
                        auto it = _symbols.find(name);
                        if (it != _symbols.end())
                        {
                            return it->second.get();
                        }
                    }

                    _ulock lock{ _lock };

                    // need to repeat due to a logical race between the check before
                    // and re-locking the lock
                    auto it = _symbols.find(name);
                    if (it != _symbols.end())
                    {
                        return it->second.get();
                    }

                    auto init_v = init();
                    auto ret = init_v.get();
                    _symbols_in_order.push_back(init_v.get());
                    _symbols.emplace(name, std::move(init_v));
                    return ret;
                }

                // this will always give you a thingy from *current* scope
                // if you want to get from any of the scopes up
                // do use resolve()
                future<symbol *> get_future(const std::u32string & name);
                future<symbol *> resolve(const std::u32string & name);

                const auto & declared_symbols() const
                {
                    assert(_is_closed);
                    return _symbols;
                }

                void set_name(std::u32string name, codegen::ir::scope_type type)
                {
                    assert(_name.empty());
                    _name = std::move(name);
                    _scope_type = type;
                }

                std::vector<codegen::ir::scope> codegen_ir(ir_generation_context & ctx) const
                {
                    std::vector<codegen::ir::scope> scopes;
                    if (_parent)
                    {
                        scopes = _parent->codegen_ir(ctx);
                    }

                    if (!_name.empty())
                    {
                        scopes.emplace_back(_name, _scope_type);
                    }
                    return scopes;
                }

                const auto & symbols_in_order() const
                {
                    assert(_is_closed);
                    return _symbols_in_order;
                }

                void keep_alive()
                {
                    assert(_parent);
                    auto inserted = _parent->_keepalive.emplace(this).second;
                    assert(inserted);
                }

            private:
                mutable std::shared_mutex _lock;

                std::u32string _name;
                codegen::ir::scope_type _scope_type;

                scope * _parent = nullptr;
                std::unordered_set<std::unique_ptr<scope>> _keepalive;
                std::unordered_map<std::u32string, std::unique_ptr<symbol>> _symbols;
                std::vector<symbol *> _symbols_in_order;
                std::unordered_map<std::u32string, future<symbol *>> _symbol_futures;
                std::unordered_map<std::u32string, manual_promise<symbol *>> _symbol_promises;
                const bool _is_local_scope = false;
                const bool _is_shadowing_boundary = false;
                bool _is_closed = false;

                optional<future<>> _close_future;
                optional<manual_promise<void>> _close_promise;
            };
        }}
    }
}

