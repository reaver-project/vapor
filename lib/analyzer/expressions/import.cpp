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

#include <numeric>

#include <boost/iostreams/device/mapped_file.hpp>

#include "vapor/analyzer/expressions/import.h"
#include "vapor/analyzer/precontex.h"
#include "vapor/analyzer/symbol.h"
#include "vapor/parser/import_expression.h"
#include "vapor/sha.h"

#include "ast.pb.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    template<typename Iter>
    std::optional<boost::filesystem::path> find_module(const boost::filesystem::path & module_path, Iter begin, Iter & end, bool source_only = false)
    {
        auto actual_path = std::accumulate(begin, end, module_path, std::divides<>());

        // inside a module directory that matches...
        if (boost::filesystem::is_directory(actual_path))
        {
            // if there is a compiled module and we are looking for more than just sources, return that
            if (!source_only && boost::filesystem::is_regular_file(actual_path / "module.vprm"))
            {
                return std::make_optional(actual_path / "module.vprm");
            }

            // else if there is a module source, return that
            if (boost::filesystem::is_regular_file(actual_path / "module.vpr"))
            {
                return std::make_optional(actual_path / "module.vpr");
            }

            // but if the directory exists, but there's no "module" file inside, throw an error
            // TODO: make this throw and not assert...
            assert(!"found a module directory, but no master module file inside...");
        }

        // if there is a non-directory compiled module that matches and we are interested in more than sources...
        if (!source_only && boost::filesystem::is_regular_file(actual_path.replace_extension(".vprm")))
        {
            return std::make_optional(actual_path);
        }

        // else if there's a non-directory module source, return that
        if (boost::filesystem::is_regular_file(actual_path.replace_extension(".vpr")))
        {
            return std::make_optional(actual_path);
        }

        // otherwise, if there's a parent, try to find its parent somehow, maybe it defines the submodule
        if (begin != end - 1)
        {
            return find_module(module_path, begin, --end, source_only);
        }

        return std::nullopt;
    }

    std::optional<boost::filesystem::path> find_module(precontext & ctx, const std::vector<std::string> & module_name, bool source_only = false)
    {
        for (auto module_path : ctx.options.module_paths())
        {
            auto end = module_name.end();

            if (auto found_module = find_module(module_path, module_name.begin(), end, source_only))
            {
                return found_module;
            }
        }

        return std::nullopt;
    }

    void compile_file(precontext & ctx, const boost::filesystem::path & path)
    {
        assert(!"unimplemented: compiling a dependent module");
    }

    entity * import_module(precontext & ctx, const std::vector<std::string> & module_name);

    entity * import_from_ast(precontext & ctx, const boost::filesystem::path & path, const std::vector<std::string> & module_name)
    {
        std::ifstream interface_file{ path.string() };
        if (!interface_file)
        {
            throw exception{ logger::error } << "couldn't open module interface file: " << path;
        }

        proto::_v1::ast ast;
        if (!ast.ParseFromIstream(&interface_file))
        {
            throw exception{ logger::error } << "couldn't parse the serialized ast from the module interface file " << path;
        }

        if (auto source_path = find_module(ctx, module_name, true))
        {
            if (static_cast<std::int64_t>(boost::filesystem::last_write_time(source_path.value())) > ast.compilation_time())
            {
                boost::iostreams::mapped_file_source source{ source_path->string() };
                auto sha256sum = sha256(source.data(), source.size());

                if (sha256sum != ast.module_source_hash())
                {
                    compile_file(ctx, source_path.value());
                    return import_module(ctx, module_name);
                }
            }
        }

        assert(!"not finished: loading a compiled dependent module");
    }

    entity * import_module(precontext & ctx, const std::vector<std::string> & module_name)
    {
        if (auto found_module = find_module(ctx, module_name))
        {
            if (found_module->extension() == ".vprm")
            {
                return import_from_ast(ctx, found_module.value(), module_name);
            }

            if (found_module->extension() == ".vpr")
            {
                compile_file(ctx, found_module.value());
                return import_module(ctx, module_name);
            }

            assert(!"some weird unknown extension found by find_module!");
        }

        assert(0);
    }

    std::unique_ptr<import_expression> preanalyze_import(precontext & ctx, const parser::import_expression & parse, scope * lex_scope, import_mode mode)
    {
        fmap(parse.module_name,
            make_overload_set(
                [&](const parser::id_expression & expr) {
                    auto module = fmap(expr.id_expression_value, [](auto && id) { return utf8(id.value.string); });
                    auto ent = import_module(ctx, module);
                    assert(ent);
                    assert(0);
                    return unit{};
                },
                [&](auto && expr) -> unit { assert(0); }));

        assert(0);
    }
}
}
