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

#include "vapor/config/pragmas.h"

#include <algorithm>

namespace reaver::vapor::config
{
inline namespace _v1
{
    template<language_pragmas Pragma>
    pragma_information_type pragma_information{};

    namespace _detail
    {
        template<std::underlying_type_t<language_pragmas>... Pragmas>
        const auto & _pragma_infos_impl(std::index_sequence<Pragmas...>)
        {
            // TODO: remove the nullptr once the array isn't empty without it anymore
            // remove the -1 in get_pragma_information() together with removing that nullptr though
            static const pragma_information_type * infos[] = { &pragma_information<static_cast<language_pragmas>(Pragmas)>..., nullptr };
            return infos;
        }

        const auto & _pragma_infos()
        {
            return _pragma_infos_impl(std::make_index_sequence<static_cast<std::underlying_type_t<language_pragmas>>(language_pragmas::last_pragma)>());
        }
    }

    const pragma_information_type & get_pragma_information(language_pragmas pragma)
    {
        return *_detail::_pragma_infos()[static_cast<std::underlying_type_t<language_pragmas>>(pragma)];
    }

    const pragma_information_type & get_pragma_information(std::string name)
    {
        auto begin = std::begin(_detail::_pragma_infos());
        auto end = std::end(_detail::_pragma_infos()) - 1;

        auto it = std::find_if(begin, end, [&](auto && elem) { return elem->name == name; });
        if (it != end)
        {
            return **it;
        }

        throw unknown_pragma{ std::move(name) };
    }
}
}
