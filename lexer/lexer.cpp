/**
 * Vapor Compiler Licence
 *
 * Copyright © 2014 Michał "Griwes" Dominiak
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

#include <reaver/unit.h>

#include "vapor/lexer.h"

using reaver::vapor::lexer::token_types;
using reaver::vapor::lexer::token_type;

std::array<std::string, +token_type::count> reaver::vapor::lexer::token_types;

static auto token_types_init = []() -> reaver::unit
{
    token_types[+token_type::identifier] = "identifier";
    token_types[+token_type::string] = "string";
    token_types[+token_type::module] = "module";
    token_types[+token_type::import] = "import";
    token_types[+token_type::auto_] = "auto";
    token_types[+token_type::dot] = ".";
    token_types[+token_type::curly_bracket_open] = "{";
    token_types[+token_type::curly_bracket_close] = "}";
    token_types[+token_type::square_bracket_open] = "[";
    token_types[+token_type::square_bracket_close] = "]";
    token_types[+token_type::round_bracket_open] = "(";
    token_types[+token_type::round_bracket_close] = ")";
    token_types[+token_type::angle_bracket_open] = "<";
    token_types[+token_type::angle_bracket_close] = ">";
    token_types[+token_type::semicolon] = ";";
    token_types[+token_type::right_shift] = ">>";
    token_types[+token_type::left_shift] = "<<";
    token_types[+token_type::map] = "->>";
    token_types[+token_type::assign] = "=";
    token_types[+token_type::block_value] = "=>";

    token_types[+token_type::none] = "<EMPTY TOKEN>";

    return {};
}();

std::unordered_map<std::string, token_type> reaver::vapor::lexer::_v1::keywords = {
    { "module", token_type::module },
    { "import", token_type::import },
    { "auto", token_type::auto_ },
};

std::unordered_map<char, token_type> reaver::vapor::lexer::_v1::symbols1 = {
    { '.', token_type::dot },
    { ',', token_type::comma },
    { '{', token_type::curly_bracket_open },
    { '}', token_type::curly_bracket_close },
    { '[', token_type::square_bracket_open },
    { ']', token_type::square_bracket_close },
    { '(', token_type::round_bracket_open },
    { ')', token_type::round_bracket_close },
    { '<', token_type::angle_bracket_open },
    { '>', token_type::angle_bracket_close },
    { ';', token_type::semicolon },
    { '=', token_type::assign },
};

std::unordered_map<char, std::unordered_map<char, token_type>> reaver::vapor::lexer::_v1::symbols2 = {
    { '<', {
        { '<', token_type::left_shift }
    } },

    { '>', {
        { '>', token_type::right_shift }
    } },

    { '=', {
        { '>', token_type::block_value }
    } }
};

std::unordered_map<char, std::unordered_map<char, std::unordered_map<char, token_type>>> reaver::vapor::lexer::_v1::symbols3 = {
    { '-', {
        { '>', {
            { '>', token_type::map }
        } }
    } },
};
