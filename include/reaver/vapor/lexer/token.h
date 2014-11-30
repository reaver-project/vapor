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

#pragma once

#include <array>
#include <unordered_map>
#include <ostream>

#include <reaver/exception.h>

#include "vapor/range.h"

namespace reaver
{
    namespace vapor
    {
        namespace lexer { inline namespace _v1
        {
            enum class token_type : std::size_t
            {
                none,
                identifier,
                string,
                string_suffix,
                integer,
                integer_suffix,
                module,
                import,
                auto_,
                return_,
                dot,
                comma,
                curly_bracket_open,
                curly_bracket_close,
                square_bracket_open,
                square_bracket_close,
                round_bracket_open,
                round_bracket_close,
                angle_bracket_open,
                angle_bracket_close,
                semicolon,
                right_shift,
                left_shift,
                map,
                assign,
                block_value,
                count                                       // always the last
            };

            constexpr std::size_t operator+(token_type type)
            {
                return static_cast<std::size_t>(type);
            }

            class invalid_suffix_requested : public exception
            {
            public:
                invalid_suffix_requested() : exception{ logger::crash }
                {
                    *this << "invalid suffix requested in lexer";
                }
            };

            constexpr token_type suffix(token_type t)
            {
                switch (t)
                {
                    case token_type::integer: return token_type::integer_suffix;
                    default: throw invalid_suffix_requested{};
                }
            }

            extern std::array<std::string, +token_type::count> token_types;

            extern std::unordered_map<std::string, token_type> keywords;
            extern std::unordered_map<char, token_type> symbols1;
            extern std::unordered_map<char, std::unordered_map<char, token_type>> symbols2;
            extern std::unordered_map<char, std::unordered_map<char, std::unordered_map<char, token_type>>> symbols3;

            class iterator;

            struct token
            {
                token() {}
                token(const token &) = default;
                token(token &&) = default;
                token & operator=(const token &) = default;
                token & operator=(token &&) = default;

                token(token_type t, std::string s, class range r) : type{ t }, string{ std::move(s) }, range{ std::move(r) }
                {
                }

                token_type type;
                std::string string;
                class range range;
            };

            bool operator==(const token & lhs, const token & rhs)
            {
                return lhs.type == rhs.type && lhs.string == rhs.string && lhs.range == rhs.range;
            }

            std::ostream & operator<<(std::ostream & os, const token & tok)
            {
                return os << "token type: `" << token_types[+tok.type] << "` token value: `" << tok.string << "` token range: " << tok.range;
            };
        }}
    }
}
