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

#include <functional>
#include <string>

#include <reaver/exception.h>

namespace reaver::vapor::config
{
inline namespace _v1
{
    // all language-level pragmas to enable (and require) language features should be defined here
    // currently empty, but I will probably start populating this once 0.1 or something ships
    // but since I'm doing the basics of CLI and need some sort of config thingy, I might as well
    // draft how this works anyway

    enum class language_pragmas
    {
        // this must be last
        // please don't make me yell at you for breaking it, whoever you are
        last_pragma
    };

    struct pragma_information_type
    {
        language_pragmas pragma_value;
        std::string_view name;
        std::string_view description;
    };

    const pragma_information_type & get_pragma_information(language_pragmas pragma);
    const pragma_information_type & get_pragma_information(std::string name);

    class unknown_pragma : public exception
    {
    public:
        unknown_pragma(std::string name_) : exception{ logger::error }, name{ std::move(name_) }
        {
            *this << "unknown language pragma: `" << name << "`";
        }

        const std::string name;
    };
}
}

template<>
struct std::hash<reaver::vapor::config::_v1::language_pragmas>
{
    std::size_t operator()(reaver::vapor::config::_v1::language_pragmas pragma) const
    {
        using underlying = std::underlying_type_t<decltype(pragma)>;
        return std::hash<underlying>()(static_cast<underlying>(pragma));
    }
};
