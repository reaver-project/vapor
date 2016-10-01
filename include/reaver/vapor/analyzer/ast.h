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

#include "../parser/ast.h"
#include "module.h"
#include "helpers.h"
#include "../codegen/ir/module.h"

namespace reaver
{
    namespace vapor
    {
        namespace analyzer { inline namespace _v1
        {
            class ast
            {
            public:
                ast(parser::ast original_ast) : _original_ast{ std::move(original_ast) }
                {
                    try
                    {
                        _modules = fmap(_original_ast, [](auto && m)
                        {
                            auto ret = std::make_unique<module>(m);
                            ret->analyze();
                            ret->simplify();
                            return ret;
                        });
                    }

                    catch (exception & e)
                    {
                        default_error_engine().push(e);
                    }

                    if (!default_error_engine())
                    {
                        default_error_engine().print(logger::default_logger());
                    }
                }

                ast(const ast &) = delete;
                ast(ast &&) = delete;

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

                std::vector<codegen::ir::module> codegen_ir() const
                {
                    return fmap(_modules, [](auto && mod) {
                        return mod->codegen_ir();
                    });
                }

            private:
                parser::ast _original_ast;
                std::vector<std::unique_ptr<module>> _modules;
            };

            std::ostream & operator<<(std::ostream & os, std::reference_wrapper<ast> tree)
            {
                for (auto && module : tree.get())
                {
                    os << "{\n";
                    module->print(os, 4);
                    os << "}\n";
                }

                return os;
            }
        }}
    }
}
