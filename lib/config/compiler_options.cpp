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

#include "vapor/config/compiler_options.h"

#include <boost/algorithm/string/replace.hpp>

#include "vapor/utf.h"

namespace reaver::vapor::config
{
inline namespace _v1
{
    void compiler_options::set_source_path(boost::filesystem::path path)
    {
        assert(!_source_path);
        _source_path = std::move(path);
    }

#define DEFINE_DIR(name, privname, ext, ...)                                                                                                                   \
    std::optional<boost::filesystem::path> compiler_options::name##_dir() const                                                                                \
    {                                                                                                                                                          \
        if (privname##_dir)                                                                                                                                    \
        {                                                                                                                                                      \
            return privname##_dir.value();                                                                                                                     \
        }                                                                                                                                                      \
                                                                                                                                                               \
        if (_output_dir)                                                                                                                                       \
        {                                                                                                                                                      \
            return _output_dir.value();                                                                                                                        \
        }                                                                                                                                                      \
                                                                                                                                                               \
        return {};                                                                                                                                             \
    }                                                                                                                                                          \
                                                                                                                                                               \
    void compiler_options::set_##name##_dir(boost::filesystem::path dir)                                                                                       \
    {                                                                                                                                                          \
        privname##_dir = std::move(dir);                                                                                                                       \
    }                                                                                                                                                          \
                                                                                                                                                               \
    boost::filesystem::path compiler_options::name##_path() const                                                                                              \
    {                                                                                                                                                          \
        if (privname##_path && privname##_path != "")                                                                                                          \
        {                                                                                                                                                      \
            return privname##_path.value();                                                                                                                    \
        }                                                                                                                                                      \
                                                                                                                                                               \
        if (auto dir = name##_dir())                                                                                                                           \
        {                                                                                                                                                      \
            assert(_module_name_path);                                                                                                                         \
            auto ret = dir.value() / _module_name_path.value();                                                                                                \
            return ret.string() + ext;                                                                                                                         \
        }                                                                                                                                                      \
                                                                                                                                                               \
        return _source_path.value().string() + ext;                                                                                                            \
    }                                                                                                                                                          \
                                                                                                                                                               \
    void compiler_options::set_##name##_path(boost::filesystem::path path)                                                                                     \
    {                                                                                                                                                          \
        assert(!privname##_path);                                                                                                                              \
        privname##_path = std::move(path);                                                                                                                     \
    }

    DEFINE_DIR(module, _module, "m", )
    DEFINE_DIR(llvm, _llvm, ".ll", )
    DEFINE_DIR(assembly, _assembly, ".asm", )
    DEFINE_DIR(object, _object, ".o", )

    boost::filesystem::path compiler_options::output_path() const
    {
        switch (_mode)
        {
            case compilation_modes::llvm_ir:
                return llvm_path();
            case compilation_modes::assembly:
                assert(!"unsupported: getting assembly output");
            case compilation_modes::object:
                assert(!"unsupported: building an object file directly");
            case compilation_modes::link:
                assert(!"unsupported: linking with vprc directly?");

            default:
                __builtin_unreachable();
        }
    }

    void compiler_options::set_output_dir(boost::filesystem::path path)
    {
        _output_dir = std::move(path);
    }

    void compiler_options::add_module_path(boost::filesystem::path path)
    {
        _module_paths.push_back(std::move(path));
    }

    void compiler_options::set_module_name(const std::u32string & name)
    {
        assert(!_module_name_path);
        auto module_name = utf8(name);
        boost::algorithm::replace_all(module_name, ".", std::string{ boost::filesystem::path::separator });
        _module_name_path = std::move(module_name);
    }

    bool compiler_options::should_generate_llvm_ir_file() const
    {
        return _llvm_path.has_value();
    }

    bool compiler_options::should_generate_assembly_file() const
    {
        return _assembly_path.has_value();
    }
}
}
