/**
 * Vapor Compiler Licence
 *
 * Copyright © 2017 Michał "Griwes" Dominiak
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
#include <unordered_set>

#include <reaver/logger.h>

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    class statement;
    class expression;

    class replacements
    {
    public:
        replacements() = default;

        replacements(const replacements &) = delete;
        replacements(replacements &&) = default;

        ~replacements();

        void add_replacement(const statement *, statement *);
        void add_replacement(const expression *, expression *);

        statement * get_replacement(const statement *);
        expression * get_replacement(const expression *);

        statement * try_get_replacement(const statement *) const;
        expression * try_get_replacement(const expression *) const;

        std::unique_ptr<statement> claim(const statement *);
        std::unique_ptr<expression> claim(const expression *);

        std::unique_ptr<statement> copy_claim(const statement *);
        std::unique_ptr<expression> copy_claim(const expression *);

    private:
        template<typename T>
        void _fix(const T *)
        {
        }

        void _fix(const expression *);

        template<typename T>
        std::unique_ptr<T> _claim_special(const T *)
        {
            return nullptr;
        }

        auto _claim_special(const statement *);

        auto _clone(const statement *);
        auto _clone(const expression *);

        std::unordered_map<statement const *, statement *> _statements = {};
        std::unordered_map<expression const *, expression *> _expressions = {};

        std::unordered_map<statement const *, std::unique_ptr<statement>> _unclaimed_statements = {};
        std::unordered_map<expression const *, std::unique_ptr<expression>> _unclaimed_expressions = {};

        std::unordered_set<statement const *> _added_statements;
        std::unordered_set<expression const *> _added_expressions;
    };
}
}
