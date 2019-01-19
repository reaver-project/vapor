/**
 * Vapor Compiler Licence
 *
 * Copyright © 2017-2018 Michał "Griwes" Dominiak
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

#include <boost/filesystem.hpp>

#include <reaver/function.h>

#include "language_options.h"

namespace reaver::vapor::config
{
inline namespace _v1
{
    namespace compilation_modes
    {
        enum compilation_modes : std::size_t
        {
            llvm_ir = 1 << 0,
            assembly = 1 << 1,
            object = 1 << 2,
            link = 1 << 3
        };
    }

    using modes_enum = compilation_modes::compilation_modes;

    using compilation_handler = unique_function<void(const boost::filesystem::path &) const>;

    class compiler_options
    {
    public:
        compiler_options(std::unique_ptr<class language_options> lang_opt) : _lang_opt{ std::move(lang_opt) }
        {
        }

        auto & language_options() const
        {
            return *_lang_opt;
        }

        const std::optional<boost::filesystem::path> & source_path() const
        {
            return _source_path;
        }

        void set_source_path(boost::filesystem::path path);

        modes_enum compilation_mode() const
        {
            return _mode;
        }

        void set_compilation_mode(modes_enum mode)
        {
            _mode = mode;
        }

        void set_compilation_handler(compilation_handler handler)
        {
            _compilation_handler = std::move(handler);
        }

        void compile_file(const boost::filesystem::path & file) const
        {
            assert(_compilation_handler);
            _compilation_handler.value()(file);
        }

#define DEFINE_DIR(name, privname)                                                                           \
public:                                                                                                      \
    std::optional<boost::filesystem::path> name##_dir() const;                                               \
    void set_##name##_dir(boost::filesystem::path dir);                                                      \
    boost::filesystem::path name##_path() const;                                                             \
    void set_##name##_path(boost::filesystem::path path);                                                    \
                                                                                                             \
private:                                                                                                     \
    std::optional<boost::filesystem::path> privname##_path;                                                  \
    std::optional<boost::filesystem::path> privname##_dir;                                                   \
                                                                                                             \
public:

        DEFINE_DIR(module, _module)
        DEFINE_DIR(llvm, _llvm)
        DEFINE_DIR(assembly, _assembly)
        DEFINE_DIR(binary, _binary)

#undef DEFINE_DIR

        boost::filesystem::path object_path() const;
        void set_output_dir(boost::filesystem::path dir);

        const std::vector<boost::filesystem::path> & module_paths() const
        {
            return _module_paths;
        }

        void add_module_path(boost::filesystem::path path);
        void add_first_module_path(boost::filesystem::path path);
        void set_module_name(const std::u32string & name);

        bool should_generate_llvm_ir_file() const;
        bool should_generate_assembly_file() const;

    private:
        std::unique_ptr<class language_options> _lang_opt;

        std::optional<compilation_handler> _compilation_handler;

        modes_enum _mode = compilation_modes::link;
        std::optional<boost::filesystem::path> _source_path;
        std::optional<boost::filesystem::path> _output_dir;
        std::vector<boost::filesystem::path> _module_paths;
        std::optional<boost::filesystem::path> _module_name_path;
    };
}
}
