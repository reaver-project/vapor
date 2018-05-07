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

#include "../codegen/ir/entity.h"
#include "../parser/ast.h"
#include "../parser/module.h"
#include "expressions/import.h"
#include "expressions/module.h"
#include "helpers.h"
#include "precontext.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    class ast
    {
    public:
        ast(parser::ast original_ast, const config::compiler_options & opts);
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

        void analyze();
        void simplify();
        std::vector<codegen::ir::entity> codegen_ir() const;

        void serialize_to(std::ostream &) const;

    private:
        parser::ast _original_ast;
        std::vector<std::unique_ptr<module>> _modules;
        std::vector<std::unique_ptr<import_expression>> _imports;

        std::unique_ptr<scope> _global_scope;
        analysis_context _proper;
        precontext _ctx;

        std::optional<boost::filesystem::path> _source_path;
    };

    std::ostream & operator<<(std::ostream & os, std::reference_wrapper<ast> tree);
}
}
