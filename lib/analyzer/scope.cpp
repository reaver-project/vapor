/**
 * Vapor Compiler Licence
 *
 * Copyright © 2016-2018 Michał "Griwes" Dominiak
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

#include <reaver/future_get.h>

#include "vapor/analyzer/expressions/call.h"
#include "vapor/analyzer/expressions/expression_ref.h"
#include "vapor/analyzer/expressions/function.h"
#include "vapor/analyzer/expressions/integer.h"
#include "vapor/analyzer/expressions/type.h"
#include "vapor/analyzer/function.h"
#include "vapor/analyzer/symbol.h"
#include "vapor/analyzer/types/sized_integer.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    scope::~scope()
    {
        close();
    }

    void scope::close()
    {
        if (_is_closed)
        {
            return;
        }

        _is_closed = true;

        if (!_is_shadowing_boundary && _parent)
        {
            _parent->close();
        }
    }

    symbol * scope::init(const std::u32string & name, std::unique_ptr<symbol> symb)
    {
        if (non_overridable().find(name) != non_overridable().end())
        {
            assert(0);
        }

        assert(!_is_closed);

        for (auto scope = this; scope; scope = scope->_parent)
        {
            if (scope->_symbols.find(name) != scope->_symbols.end())
            {
                return nullptr;
            }

            if (scope->_is_shadowing_boundary)
            {
                break;
            }
        }

        _symbols_in_order.push_back(symb.get());
        return _symbols.emplace(name, std::move(symb)).first->second.get();
    }

    symbol * scope::get(const std::u32string & name) const
    {
        auto symb = try_get(name);
        if (!symb)
        {
            throw failed_lookup{ name };
        }
        return symb.value();
    }

    std::optional<symbol *> scope::try_get(const std::u32string & name) const
    {
        auto it = _symbols.find(name);
        if (it == _symbols.end() || it->second->is_hidden())
        {
            return std::nullopt;
        }

        return std::make_optional(it->second.get());
    }

    symbol * scope::resolve(const std::u32string & name) const
    {
        {
            auto it = non_overridable().find(name);
            if (it != non_overridable().end())
            {
                return it->second.get();
            }
        }

        auto it = _resolve_cache.find(name);
        if (it != _resolve_cache.end())
        {
            return it->second;
        }

        auto scope = this;

        while (scope)
        {
            auto symb = scope->try_get(name);

            if (symb && !symb.value()->is_hidden())
            {
                _resolve_cache.emplace(name, symb.value());
                return symb.value();
            }

            scope = scope->_parent;
        }

        throw failed_lookup(name);
    }

    const std::unordered_map<std::u32string, std::unique_ptr<symbol>> & non_overridable()
    {
        static auto integer_type_expr = builtin_types().integer->get_expression();
        static auto boolean_type_expr = builtin_types().boolean->get_expression();
        static auto type_type_expr = builtin_types().type->get_expression();

        static auto sized_int = [] {
            auto ret = make_function("sized_int");
            ret->set_return_type(builtin_types().type->get_expression());
            ret->set_parameters({ integer_type_expr });

            ret->add_analysis_hook([](analysis_context & ctx, call_expression * expr, std::vector<expression *> args) {
                assert(args.size() == 2 && args[1]->get_type() == builtin_types().integer.get());
                auto int_var = static_cast<integer_constant *>(args[1]);
                auto size = int_var->get_value().convert_to<std::size_t>();

                auto & type = ctx.sized_integers[size];
                if (!type)
                {
                    type = make_sized_integer_type(size);
                }

                expr->replace_with(make_expression_ref(type->get_expression()));

                return make_ready_future();
            });

            return ret;
        }();

        static auto sized_int_expr = std::unique_ptr<expression>{ reaver::get(make_function_expression(sized_int.get())) };

        static auto symbols = [&] {
            std::unordered_map<std::u32string, std::unique_ptr<symbol>> symbols;

            auto add_symbol = [&](auto name, auto && expr) { symbols.emplace(name, make_symbol(name, expr)); };

            add_symbol(U"int", integer_type_expr);
            add_symbol(U"bool", boolean_type_expr);
            add_symbol(U"type", type_type_expr);

            add_symbol(U"sized_int", sized_int_expr.get());

            return symbols;
        }();

        return symbols;
    }
}
}
