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

#include "vapor/config/language_options.h"

namespace reaver::vapor::config
{
inline namespace _v1
{
    language_options::language_options(std::unordered_set<language_pragmas> set) : _pragmas{ std::move(set) }
    {
    }

    bool language_options::is_pragma_enabled(language_pragmas pragma) const
    {
        return _pragmas.count(pragma);
    }

    bool language_options::is_pragma_enabled(std::string pragma) const
    {
        return is_pragma_enabled(get_pragma_information(std::move(pragma)).pragma_value);
    }

    void language_options::enable_pragma(language_pragmas pragma)
    {
        _pragmas.insert(pragma);
    }

    void language_options::enable_pragma(std::string pragma)
    {
        enable_pragma(get_pragma_information(std::move(pragma)).pragma_value);
    }
}
}
