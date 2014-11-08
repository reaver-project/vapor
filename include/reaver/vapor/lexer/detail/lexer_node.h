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

#include <memory>

#include <reaver/semaphore.h>
#include <reaver/error.h>

#include "vapor/lexer/token.h"

namespace reaver
{
    namespace vapor
    {
        namespace lexer { inline namespace _v1
        {
            class iterator;

            namespace _detail
            {
                class _iterator_backend;

                class _lexer_node
                {
                public:
                    friend class lexer::iterator;
                    friend class _iterator_backend;

                    _lexer_node(token tok, error_engine & engine) : _token{ std::move(tok) }, _engine(engine)
                    {
                    }

                    void wait_next()
                    {
                        if (_done)
                        {
                            return;
                        }

                        if (!_engine)
                        {
                            throw std::move(_engine);
                        }

                        _sem.wait();

                        if (!_engine)
                        {
                            throw std::move(_engine);
                        }
                    }

                private:
                    std::shared_ptr<_lexer_node> _next;
                    token _token;
                    semaphore _sem;
                    error_engine & _engine;
                    std::atomic<bool> _done{ false };
                };
            }
        }}
    }
}
