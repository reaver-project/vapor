/**
 * Vapor Compiler Licence
 *
 * Copyright © 2014-2016 Michał "Griwes" Dominiak
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
#include <type_traits>

#include <reaver/exception.h>
#include <reaver/relaxed_constexpr.h>

#include "../range.h"
#include "../utf8.h"

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
                boolean,

                module,
                import,
                auto_,
                let,
                return_,
                function,

                if_,
                else_,

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
                colon,
                semicolon,
                map,
                bind,
                indirection,
                assign,
                block_value,

                logical_not,
                bitwise_not,
                bitwise_not_assignment,

                plus,
                minus,
                star,
                slash,
                modulo,
                bitwise_and,
                bitwise_or,
                bitwise_xor,
                logical_and,
                logical_or,
                right_shift,
                left_shift,

                plus_assignment,
                minus_assignment,
                star_assignment,
                slash_assignment,
                modulo_assignment,
                bitwise_and_assignment,
                bitwise_or_assignment,
                bitwise_xor_assignment,
                logical_and_assignment,
                logical_or_assignment,
                right_shift_assignment,
                left_shift_assignment,

                equals,
                not_equals,
                less_equal,
                greater_equal,

                increment,
                decrement,

                lambda,

                count,                                      // always the last, except for duplicates

                less = angle_bracket_open,
                greater = angle_bracket_close
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

            inline relaxed_constexpr token_type suffix(token_type t)
            {
                switch (t)
                {
                    case token_type::integer: return token_type::integer_suffix;
                    default: throw invalid_suffix_requested{};
                }
            }

            extern std::array<std::string, +token_type::count> token_types;

            extern const std::unordered_map<std::u32string, token_type> keywords;
            extern const std::unordered_map<char32_t, token_type> symbols1;
            extern const std::unordered_map<char32_t, std::unordered_map<char32_t, token_type>> symbols2;
            extern const std::unordered_map<char32_t, std::unordered_map<char32_t, std::unordered_map<char32_t, token_type>>> symbols3;

            class iterator;

            struct token
            {
                token() {}
                token(const token &) = default;
                token(token &&) = default;
                token & operator=(const token &) = default;
                token & operator=(token &&) = default;

                token(token_type t, std::u32string s, range_type r) : type{ t }, string{ std::move(s) }, range{ std::move(r) }
                {
                }

                token_type type;
                std::u32string string;
                range_type range;
            };

            inline bool operator==(const token & lhs, const token & rhs)
            {
                return lhs.type == rhs.type && lhs.string == rhs.string && lhs.range == rhs.range;
            }

            inline std::ostream & operator<<(std::ostream & os, const token & tok)
            {
                return os << "token type: `" << token_types[+tok.type] << "` token value: `" << utf8(tok.string) << "` token range: " << tok.range;
            };
        }}
    }
}

namespace std
{
    template<>
    struct hash<::reaver::vapor::lexer::token_type>
    {
        using argument_type = ::reaver::vapor::lexer::token_type;
        using underlying_type = std::underlying_type<argument_type>::type;
        using result_type = std::hash<underlying_type>::result_type;
        result_type operator()(const argument_type & arg) const
        {
            std::hash<underlying_type> hasher;
            return hasher(static_cast<underlying_type>(arg));
        }
    };
}

