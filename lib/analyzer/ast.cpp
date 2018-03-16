/**
 * Vapor Compiler Licence
 *
 * Copyright © 2018 Michał "Griwes" Dominiak
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

#include "vapor/analyzer/ast.h"

#include <boost/iostreams/device/mapped_file.hpp>

#include "vapor/sha.h"
#include "vapor/parser/expr.h"

#include "ast.pb.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    ast::ast(parser::ast original_ast, const config::compiler_options & opts)
        : _original_ast{ std::move(original_ast) }, _global_scope{ std::make_unique<scope>() }, _source_path{ opts.source_path() }
    {
        try
        {
            precontext ctx{ opts };

            _imports = fmap(
                _original_ast.global_imports, [this, &ctx](auto && im) { return preanalyze_import(ctx, im, _global_scope.get(), import_mode::statement); });
            _modules = fmap(_original_ast.module_definitions, [this, &ctx](auto && m) { return preanalyze_module(ctx, m, _global_scope.get()); });
        }

        catch (exception & e)
        {
            default_error_engine().push(e);
        }

        default_error_engine().validate();
    }

    void ast::analyze()
    {
        get(when_all(fmap(_modules, [this](auto && m) { return m->analyze(_ctx); })));
    }

    void ast::simplify()
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

    std::vector<codegen::ir::module> ast::codegen_ir() const
    {
        ir_generation_context ctx;

        return mbind(_modules, [&](auto && mod) { return fmap(mod->declaration_codegen_ir(ctx), [](auto && ir) { return get<codegen::ir::module>(ir); }); });
    }

    std::ostream & operator<<(std::ostream & os, std::reference_wrapper<ast> tree)
    {
        for (auto && module : tree.get())
        {
            module->print(os, {});
        }

        return os;
    }

    void ast::serialize_to(std::ostream & os) const
    {
        proto::ast serialized;

        auto info = std::make_unique<proto::compilation_information>();

        auto time = std::chrono::system_clock::now().time_since_epoch();
        info->set_time(std::chrono::duration_cast<std::chrono::seconds>(time).count());

        boost::iostreams::mapped_file_source source{ _source_path->string() };
        auto sha256sum = sha256(source.data(), source.size());
        info->set_source_hash(sha256sum);

        serialized.set_allocated_compilation_info(info.release());

        for (auto && import : _imports)
        {
            import->generate_interface(*serialized.add_imports());
        }

        for (auto && module : _modules)
        {
            module->generate_interface(*serialized.add_modules());
        }

        serialized.SerializeToOstream(&os);
    }
}
}
