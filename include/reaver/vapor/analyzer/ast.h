/**
 * Vapor Compiler Licence
 *
 * Copyright © 2014, 2016-2018 Michał "Griwes" Dominiak
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

#include <reaver/future_get.h>

#include "../codegen/ir/module.h"
#include "../parser/ast.h"
#include "../parser/module.h"
#include "helpers.h"
#include "module.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    class ast
    {
    public:
        ast(parser::ast original_ast) : _original_ast{ std::move(original_ast) }, _global_scope{ std::make_unique<scope>() }
        {
            try
            {
                assert(_original_ast.global_imports.empty());
                _modules = fmap(_original_ast.module_definitions, [this](auto && m) { return preanalyze_module(m, _global_scope.get()); });
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

        void analyze()
        {
            get(when_all(fmap(_modules, [this](auto && m) { return m->analyze(_ctx); })));
        }

        void simplify()
        {
            bool cont = true;
            cached_results res;

            while (cont)
            {
                simplification_context ctx{ res };
                get(when_all(fmap(_modules, [&ctx](auto && m) { return m->simplify_module({ ctx }); })));

                cont = ctx.did_something_happen();
            }
        }

        std::vector<codegen::ir::module> codegen_ir() const
        {
            ir_generation_context ctx;

            return mbind(
                _modules, [&](auto && mod) { return fmap(mod->declaration_codegen_ir(ctx), [](auto && ir) { return get<codegen::ir::module>(ir); }); });
        }

    private:
        parser::ast _original_ast;
        std::vector<std::unique_ptr<module>> _modules;

        std::unique_ptr<scope> _global_scope;
        analysis_context _ctx;
    };

    inline std::ostream & operator<<(std::ostream & os, std::reference_wrapper<ast> tree)
    {
        for (auto && module : tree.get())
        {
            module->print(os, {});
        }

        return os;
    }
}
}
