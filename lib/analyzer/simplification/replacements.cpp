/**
 * Vapor Compiler Licence
 *
 * Copyright © 2017, 2019 Michał "Griwes" Dominiak
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

#include "vapor/analyzer/simplification/replacements.h"
#include "vapor/analyzer/expressions/expression.h"
#include "vapor/analyzer/semantic/symbol.h"
#include "vapor/analyzer/statements/statement.h"

#define GENERATE(X)                                                                                          \
    void replacements::add_replacement(const X * original, X * repl)                                         \
    {                                                                                                        \
        _added_##X##s.emplace(original);                                                                     \
                                                                                                             \
        if (original == repl)                                                                                \
        {                                                                                                    \
            return;                                                                                          \
        }                                                                                                    \
                                                                                                             \
        assert(_##X##s.count(original) == 0);                                                                \
        logger::dlog(logger::trace) << "replacements @ " << this << ": add replacement " #X ": " << original \
                                    << " => " << repl;                                                       \
                                                                                                             \
        auto & repls = _##X##s;                                                                              \
        repls.emplace(original, repl);                                                                       \
                                                                                                             \
        _fix(original);                                                                                      \
    }                                                                                                        \
                                                                                                             \
    X * replacements::get_replacement(const X * ptr)                                                         \
    {                                                                                                        \
        auto & repls = _##X##s;                                                                              \
                                                                                                             \
        auto & repl = repls[ptr];                                                                            \
        if (!repl)                                                                                           \
        {                                                                                                    \
            auto & unclaimeds = _unclaimed_##X##s;                                                           \
            auto clone = _clone(ptr);                                                                        \
            repl = clone.get();                                                                              \
            unclaimeds.emplace(ptr, std::move(clone));                                                       \
                                                                                                             \
            logger::dlog(logger::trace)                                                                      \
                << "replacements @ " << this << ": add replacement " #X ": " << ptr << " => " << repl;       \
                                                                                                             \
            _fix(ptr);                                                                                       \
        }                                                                                                    \
                                                                                                             \
        return repl;                                                                                         \
    }                                                                                                        \
                                                                                                             \
    X * replacements::try_get_replacement(const X * ptr) const                                               \
    {                                                                                                        \
        auto it = _##X##s.find(ptr);                                                                         \
        if (it != _##X##s.end())                                                                             \
        {                                                                                                    \
            return it->second;                                                                               \
        }                                                                                                    \
                                                                                                             \
        return nullptr;                                                                                      \
    }                                                                                                        \
                                                                                                             \
    std::unique_ptr<X> replacements::claim(const X * ptr)                                                    \
    {                                                                                                        \
        get_replacement(ptr);                                                                                \
                                                                                                             \
        auto possible = _claim_special(ptr);                                                                 \
        if (possible)                                                                                        \
        {                                                                                                    \
            return possible;                                                                                 \
        }                                                                                                    \
                                                                                                             \
        auto & unclaimed = _unclaimed_##X##s;                                                                \
        assert(_unclaimed_##X##s.count(ptr));                                                                \
        auto ret = std::move(unclaimed.at(ptr));                                                             \
        unclaimed.erase(ptr);                                                                                \
        return ret;                                                                                          \
    }                                                                                                        \
                                                                                                             \
    std::unique_ptr<X> replacements::copy_claim(const X * ptr)                                               \
    {                                                                                                        \
        auto repl = try_get_replacement(ptr);                                                                \
        if (repl)                                                                                            \
        {                                                                                                    \
            if (_unclaimed_##X##s.count(ptr))                                                                \
            {                                                                                                \
                return claim(ptr);                                                                           \
            }                                                                                                \
            return claim(repl);                                                                              \
        }                                                                                                    \
                                                                                                             \
        return claim(ptr);                                                                                   \
    }

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    replacements::~replacements()
    {
#define CLEAR_ADDED(X)                                                                                       \
    for (auto && added : _added_##X)                                                                         \
    {                                                                                                        \
        _unclaimed_##X.erase(added);                                                                         \
    }

        CLEAR_ADDED(statements);
        CLEAR_ADDED(expressions);

#undef CLEAR_ADDED

        assert(_unclaimed_statements.empty());
        assert(_unclaimed_expressions.empty());
    }

    void replacements::_fix(const expression * ptr)
    {
        _statements.emplace(ptr, _expressions[ptr]);
    }

    auto replacements::_claim_special(const statement * ptr)
    {
        if (!dynamic_cast<const expression *>(ptr))
        {
            return std::unique_ptr<expression>();
        }

        auto it = _unclaimed_expressions.find(static_cast<const expression *>(ptr));
        if (it != _unclaimed_expressions.end())
        {
            auto ret = std::move(it->second);
            _unclaimed_expressions.erase(it);
            return ret;
        }

        return std::unique_ptr<expression>();
    }

    auto replacements::_clone(const statement * ptr)
    {
        auto ret = ptr->clone_with_replacement(*this);
        logger::dlog(logger::trace) << "[" << this << "] Clone for " << ptr << " is " << ret.get();
        return ret;
    }

    auto replacements::_clone(const expression * ptr)
    {
        auto ret = ptr->clone_expr_with_replacement(*this);
        logger::dlog(logger::trace) << "[" << this << "] Clone for " << ptr << " is " << ret.get();
        return ret;
    }

    GENERATE(statement);
    GENERATE(expression);
}
}
