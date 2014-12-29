/**
 * Vapor Compiler Licence
 *
 * Copyright © 2014 Michał "Griwes" Dominiak
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

#include "vapor/parser/ast.h"
#include "vapor/analyzer/module.h"
#include "vapor/analyzer/helpers.h"

namespace reaver
{
    namespace vapor
    {
        namespace analyzer { inline namespace v1
        {
            class ast
            {
            public:
                ast(const parser::ast & original_ast)
                {
                    try
                    {
                        std::transform(original_ast.begin(), original_ast.end(), std::back_inserter(_modules), [](auto && m)
                        {
                            auto ret = std::make_shared<module>(m);
                            ret->analyze();
                            return ret;
                        });
                    }

                    catch (exception & e)
                    {
                        default_error_engine().push(e);
                    }

                    if (default_error_engine().size())
                    {
                        default_error_engine().print(logger::default_logger());
                    }
                }

                auto begin()
                {
                    return _modules.begin();
                }

                auto begin() const
                {
                    return _modules.begin();
                }

                auto end()
                {
                    return _modules.end();
                }

                auto end() const
                {
                    return _modules.end();
                }

            private:
                std::vector<std::shared_ptr<module>> _modules;
            };
        }}
    }
}
