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

#pragma once

#include <memory>
#include <ostream>
#include <string>

#include "../utf.h"

#include "ir/entity.h"

namespace reaver::vapor::codegen
{
inline namespace _v1
{
    class code_generator;

    class result
    {
    public:
        result(std::vector<codegen::ir::entity>, std::shared_ptr<code_generator>);

        friend std::ostream & operator<<(std::ostream & os, const result & res)
        {
            os << utf8(res._generated_code) << '\n';
            return os;
        }

    private:
        std::u32string _generated_code;
    };
}
}
