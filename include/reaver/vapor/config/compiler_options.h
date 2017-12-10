/**
 * Vapor Compiler Licence
 *
 * Copyright © 2017 Michał "Griwes" Dominiak
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

#include "language_options.h"

namespace reaver::vapor::config
{
inline namespace _v1
{
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

        boost::filesystem::path output_path() const
        {
            if (_output_path)
            {
                return _output_path.value();
            }

            return _source_path.value().string() + ".ll";
        }

        const std::vector<boost::filesystem::path> & module_paths() const
        {
            return _module_paths;
        }

        void set_source_path(boost::filesystem::path path)
        {
            assert(!_source_path);
            _source_path = std::move(path);
        }

        void set_output_path(boost::filesystem::path path)
        {
            assert(!_output_path);
            _output_path = std::move(path);
        }

        void add_module_path(boost::filesystem::path path)
        {
            _module_paths.push_back(std::move(path));
        }

    private:
        std::unique_ptr<class language_options> _lang_opt;

        std::optional<boost::filesystem::path> _source_path;
        std::optional<boost::filesystem::path> _output_path;
        std::vector<boost::filesystem::path> _module_paths;
    };
}
}
