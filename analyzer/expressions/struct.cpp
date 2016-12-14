/**
 * Vapor Compiler Licence
 *
 * Copyright © 2016 Michał "Griwes" Dominiak
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

#include "vapor/analyzer/expressions/struct.h"
#include "vapor/analyzer/symbol.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    struct_literal::struct_literal(const parser::struct_literal & parse, scope * lex_scope) : _parse{ parse }, _type{ make_struct_type(parse, lex_scope) }
    {
    }

    void struct_literal::print(std::ostream & os, std::size_t indent) const
    {
        auto in = std::string(indent, ' ');

        os << in << "structure at " << _parse.range << '\n';
        os << in << "members:\n";
        os << in << "{\n";
        os << in << "TODO: print this shit\n";
        os << in << "}\n";
    }

    statement_ir struct_literal::_codegen_ir(ir_generation_context & ctx) const
    {
        return {};
    }
}
}
