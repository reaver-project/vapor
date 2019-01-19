/**
 * Vapor Compiler Licence
 *
 * Copyright © 2018 Michał "Griwes" Dominiak
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

#include <string>

#include <sodium.h>

namespace reaver::vapor
{
inline namespace _v1
{
    inline std::string sha256(const char * src, std::size_t size)
    {
        std::string out(crypto_hash_sha256_BYTES, '\0');
        crypto_hash_sha256(reinterpret_cast<unsigned char *>(out.data()),
            reinterpret_cast<const unsigned char *>(src),
            size);
        return out;
    }
}
}
