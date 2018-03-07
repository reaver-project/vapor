/**
 * Vapor Compiler Licence
 *
 * Copyright © 2014, 2016, 2018 Michał "Griwes" Dominiak
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

#include <memory>
#include <type_traits>

#include "detail/iterator_backend.h"

namespace reaver::vapor::lexer
{
inline namespace _v1
{
    class iterator
    {
    public:
        iterator() = default;

        template<typename Iter, typename std::enable_if<std::is_same<typename std::iterator_traits<Iter>::value_type, char32_t>::value, int>::type = 0>
        iterator(Iter begin, Iter end, std::optional<std::string_view> filename)
            : _backend{ std::make_shared<_detail::_iterator_backend>(begin, end, _node, filename) }
        {
        }

        explicit operator bool() const
        {
            return _node != nullptr;
        }

        bool operator!=(const iterator & other) const
        {
            return _node != other._node;
        }

        iterator & operator++()
        {
            if (_node)
            {
                if (!_node->_next)
                {
                    _node->wait_next();
                }

                _node = _node->_next;
            }

            return *this;
        }

        iterator operator++(int)
        {
            iterator iter{ *this };
            ++*this;
            return iter;
        };

        token & operator*()
        {
            return _node->_token;
        }

        const token & operator*() const
        {
            return _node->_token;
        }

        token * operator->()
        {
            return &_node->_token;
        }

        const token * operator->() const
        {
            return &_node->_token;
        }

        bool operator==(const iterator & rhs) const
        {
            // throw assert _backend == rhs._backend
            return _node == rhs._node;
        }

    private:
        std::shared_ptr<_detail::_lexer_node> _node = nullptr;
        std::shared_ptr<_detail::_iterator_backend> _backend = nullptr;
    };
}
}

namespace std
{
template<>
struct iterator_traits<reaver::vapor::lexer::iterator>
{
    using value_type = reaver::vapor::lexer::token;
    using iterator_category = forward_iterator_tag;
};
}
