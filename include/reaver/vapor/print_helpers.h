/**
 * Vapor Compiler Licence
 *
 * Copyright © 2017-2019 Michał "Griwes" Dominiak
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

#include <optional>

#include <reaver/style.h>

#include "range.h"
#include "utf.h"

namespace reaver::vapor
{
namespace analyzer
{
    inline namespace _v1
    {
        struct precontext;
        class expression;
    }
}
namespace proto
{
    struct range;
}
inline namespace _v1
{
    namespace styles
    {
        static const style::style def = {};
        static const style::style rule_name = { style::colors::bgreen,
            style::colors::def,
            style::styles::bold };
        static const style::style subrule_name = { style::colors::bgreen, style::colors::def };
        static const style::style range = { style::colors::brown };
        static const style::style string_value = { style::colors::green };
        static const style::style address = { style::colors::red };
        static const style::style type = { style::colors::bmagenta, style::colors::def, style::styles::bold };
    }

    class print_context
    {
    public:
        print_context() = default;

        print_context(std::string_view prefix, std::string_view indent) : _prefix{ prefix }, _indent{ indent }
        {
        }

        print_context make_branch(bool is_last) const
        {
            if (is_last)
            {
                if (!_own_last_branch)
                {
                    _own_last_branch = std::string(_indent) + last_branch;
                    _own_last_branch_indent = std::string(_indent) + indent;
                }

                return { *_own_last_branch, *_own_last_branch_indent };
            }

            if (!_own_branch)
            {
                _own_branch = std::string(_indent) + branch;
                _own_branch_indent = std::string(_indent) + nested_indent;
            }

            return { *_own_branch, *_own_branch_indent };
        }

        std::string_view prefix() const
        {
            return _prefix;
        }

    private:
        static constexpr const char * nothing = "";
        static constexpr const char * branch = "├── ";
        static constexpr const char * last_branch = "└── ";
        static constexpr const char * indent = "    ";
        static constexpr const char * nested_indent = "│   ";

        mutable std::optional<std::string> _own_branch;
        mutable std::optional<std::string> _own_branch_indent;
        mutable std::optional<std::string> _own_last_branch;
        mutable std::optional<std::string> _own_last_branch_indent;
        std::string_view _prefix = nothing;
        std::string_view _indent = nothing;
    };

    inline std::ostream & operator<<(std::ostream & os, const print_context & ctx)
    {
        return os << ctx.prefix();
    }

    // parser
    template<typename T>
    void print_address_range(std::ostream & os, const T & ref)
    {
        os << styles::def << " @ " << styles::address << &ref;
        os << styles::def << " (" << styles::range << ref.range << styles::def << "):";
    }

    // analyzer
    struct ast_node
    {
        const void * address;
        range_type range;
    };

    ast_node imported_ast_node(analyzer::precontext & ctx, const proto::range & range);

    template<typename T>
    auto make_node(const T & t)
    {
        return ast_node{ &t, t.range };
    }

    inline void print_address_range(std::ostream & os, const ast_node & ref)
    {
        os << styles::def << " @ " << styles::address << ref.address;
        os << styles::def << " (" << styles::range << ref.range << styles::def << "):";
    }

    template<typename T>
    void print_address_range(std::ostream & os, const T * ptr)
    {
        os << styles::def << " @ " << styles::address << ptr;
        if (auto parse = ptr->get_ast_info())
        {
            os << styles::def << ", AST node";
            print_address_range(os, parse.value());
        }
    }
}
}
