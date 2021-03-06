/**
 * Vapor Compiler Licence
 *
 * Copyright © 2016-2019 Michał "Griwes" Dominiak
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

#include <numeric>

#include <boost/algorithm/string.hpp>

#include "../../parser/literal.h"
#include "../semantic/symbol.h"
#include "expression_ref.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    class identifier : public expression_ref
    {
    public:
        identifier(std::u32string name, scope * lex_scope, ast_node parse_info)
            : _lex_scope{ lex_scope }, _name{ std::move(name) }
        {
            _set_ast_info(parse_info);
        }

        const std::u32string & name() const
        {
            return _name;
        }

        virtual void print(std::ostream & os, print_context ctx) const override;

    private:
        virtual future<> _analyze(analysis_context &) override;

        scope * _lex_scope;
        std::u32string _name;
    };

    struct precontext;

    inline std::unique_ptr<identifier> preanalyze_identifier(precontext &,
        const parser::identifier & parse,
        scope * lex_scope)
    {
        return std::make_unique<identifier>(parse.value.string, lex_scope, make_node(parse));
    }
}
}
