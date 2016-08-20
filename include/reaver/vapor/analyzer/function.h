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

#pragma once

#include <memory>
#include <vector>

#include <reaver/function.h>

#include "../parser/function.h"

namespace reaver
{
    namespace vapor
    {
        namespace analyzer { inline namespace _v1
        {
            class type;

            class function
            {
            public:
                function(std::shared_ptr<type> ret, std::vector<std::shared_ptr<type>> args, reaver::function<void ()> generator)
                    : _return_type{ std::move(ret) }, _argument_types{ std::move(args) }, _generator{ std::move(generator) }
                {
                }

                std::shared_ptr<type> return_type()
                {
                    return _return_type;
                }

            private:
                optional<const parser::function &> parse;

                std::shared_ptr<type> _return_type;
                std::vector<std::shared_ptr<type>> _argument_types;
                optional<reaver::function<void ()>> _generator;
            };

            std::shared_ptr<function> make_function(std::shared_ptr<type> return_type, std::vector<std::shared_ptr<type>> arguments, reaver::function<void ()> generator)
            {
                return std::make_shared<function>(std::move(return_type), std::move(arguments), std::move(generator));
            }
        }}
    }
}

