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

#include <boost/algorithm/string/join.hpp>
#include <boost/iostreams/device/mapped_file.hpp>

#include "vapor/analyzer/expressions/import.h"
#include "vapor/analyzer/precontext.h"
#include "vapor/analyzer/symbol.h"
#include "vapor/analyzer/types/module.h"
#include "vapor/analyzer/types/unresolved.h"
#include "vapor/codegen/ir/scope.h"
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
        throw exception{ logger::error } << "unimplemented: compiling a module dependency: " << path;
    }

    entity * import_module(precontext & ctx, const std::vector<std::string> & module_name);

    void import_from_ast(precontext & ctx, const boost::filesystem::path & path, const std::vector<std::string> & module_name)
    {
        std::ifstream interface_file{ path.string() };
        if (!interface_file)
        {
            throw exception{ logger::fatal } << "couldn't open module interface file: " << path;
        }

        proto::ast ast;
        if (!ast.ParseFromIstream(&interface_file))
        {
            throw exception{ logger::fatal } << "couldn't parse the serialized ast from the module interface file " << path;
        }
        if (!ast.has_compilation_info() || ast.modules_size() == 0)
        {
            throw exception{ logger::fatal } << "no valid serialized ast in the module interface file " << path;
        }

        if (auto source_path = find_module(ctx, module_name, true))
        {
            if (static_cast<std::int64_t>(boost::filesystem::last_write_time(source_path.value())) > ast.compilation_info().time())
            {
                boost::iostreams::mapped_file_source source{ source_path->string() };
                auto sha256sum = sha256(source.data(), source.size());

                if (sha256sum != ast.compilation_info().source_hash())
                {
                    compile_file(ctx, source_path.value());
                    import_module(ctx, module_name);
                    return;
                }
            }
        }

        ctx.current_file.push(boost::filesystem::canonical(path));

        if (ast.imports_size())
        {
            throw exception{ logger::fatal } << "not implemented yet: loading a module with imports";
        }

        for (auto && module : ast.modules())
        {
            // TODO: support submodules (including the scope creation below...)
            assert(module.name().size() == 1);

            auto name = boost::algorithm::join(module.name(), ".");
            ctx.current_scope.push(name);

            auto scope = ctx.global_scope->clone_for_class();
            scope->set_name(utf32(name), codegen::ir::scope_type::module);
            ctx.module_scope = scope.get();
            auto type = std::make_unique<module_type>(std::move(scope), name);

            // seems that protobuf's map doesn't have a deterministic order of iteration
            // we can't let that happen in a compiler...
            std::vector<std::pair<const std::string *, const proto::entity *>> symbols;
            symbols.reserve(module.symbols().size());
            for (auto && entity : module.symbols())
            {
                symbols.emplace_back(&entity.first, &entity.second);
            }
            std::sort(symbols.begin(), symbols.end(), [](auto && lhs, auto && rhs) { return *lhs.first < *rhs.first; });

            for (auto && imported_entity : symbols)
            {
                if (imported_entity.second->is_associated())
                {
                    assert(imported_entity.second->associated_entities_size() == 0);
                    continue;
                }

                std::map<std::string, const proto::entity *> associated;
                for (auto && assoc : imported_entity.second->associated_entities())
                {
                    associated.emplace(assoc, &module.symbols().at(assoc));
                }

                ctx.current_symbol = *imported_entity.first;
                auto ent = get_entity(ctx, *imported_entity.second, associated);
                ent->set_name(utf32(*imported_entity.first));
                type->add_symbol(*imported_entity.first, ent.get());
                for (auto && assoc : ent->get_associated())
                {
                    type->add_symbol(assoc.first, assoc.second);
                    assoc.second->set_name(utf32(assoc.first));
                }

                ctx.imported_entities.insert(std::move(ent));
            }

            auto & saved = ctx.loaded_modules[name];
            assert(!saved);
            saved = make_entity(std::move(type));

            saved->set_timestamp(ast.compilation_info().time());
            saved->set_source_hash(ast.compilation_info().source_hash());
            saved->set_import_name({ module.name().begin(), module.name().end() });

            ctx.current_scope.pop();
        }

        ctx.current_file.pop();
    }

    entity * import_module(precontext & ctx, const std::vector<std::string> & module_name)
    {
        auto return_cached = [&]() -> entity * {
            auto name = boost::algorithm::join(module_name, ".");
            auto it = ctx.loaded_modules.find(name);
            if (it != ctx.loaded_modules.end())
            {
                return it->second.get();
            }

            return nullptr;
        };

        if (auto cached = return_cached())
        {
            return cached;
        }

        if (auto found_module = find_module(ctx, module_name))
        {
            if (found_module->extension() == ".vprm")
            {
                import_from_ast(ctx, found_module.value(), module_name);
                auto cached = return_cached();
                assert(cached);
                return cached;
            }

            if (found_module->extension() == ".vpr")
            {
                compile_file(ctx, found_module.value());
                return import_module(ctx, module_name);
            }

            assert(!"some weird unknown extension found by find_module!");
        }

        throw exception{ logger::error } << "couldn't find module `" << boost::algorithm::join(module_name, ".") << "`";
    }

    std::unique_ptr<import_expression> preanalyze_import(precontext & ctx, const parser::import_expression & parse, scope * lex_scope, import_mode mode)
    {
        auto expr = std::get<0>(fmap(parse.module_name,
            make_overload_set(
                [&](const parser::id_expression & expr) {
                    auto module = fmap(expr.id_expression_value, [](auto && id) { return utf8(id.value.string); });
                    auto ent = import_module(ctx, module);
                    assert(ent);

                    if (mode == import_mode::statement)
                    {
                        auto scope = lex_scope;

                        for (auto it = expr.id_expression_value.begin(); it != expr.id_expression_value.end() - 1; ++it)
                        {
                            auto sub = lex_scope->get_or_init(
                                it->value.string, [&]() -> std::unique_ptr<symbol> { assert(!"need to properly implement imports of nested modules!"); });
                            assert(sub->get_type() && dynamic_cast<module_type *>(sub->get_type()));
                            scope = sub->get_type()->get_scope();
                        }

                        scope->init(expr.id_expression_value.back().value.string, make_symbol(expr.id_expression_value.back().value.string, ent));
                    }

                    return std::make_unique<import_expression>(make_node(parse), ent);
                },
                [&](auto && expr) -> std::unique_ptr<import_expression> { assert(0); })));

        return expr;
    }

    void import_expression::print(std::ostream &, print_context) const
    {
        assert(0);
    }
}
}
