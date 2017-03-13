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

#include <reaver/mayfly.h>

#include "../helpers.h"

using namespace reaver::vapor;
using namespace reaver::vapor::analyzer;

MAYFLY_BEGIN_SUITE("analyzer");
MAYFLY_BEGIN_SUITE("variables");
MAYFLY_BEGIN_SUITE("variable");

MAYFLY_ADD_TESTCASE("default value", [] {
    test_type t1{};

    test_variable var{ &t1 };
    test_expression expr{ &t1 };

    MAYFLY_REQUIRE(var.get_default_value() == nullptr);

    var.set_default_value(&expr);
    MAYFLY_REQUIRE(var.get_default_value() == &expr);
});

MAYFLY_ADD_TESTCASE("clone cache", [] {
    test_type t1{};
    test_variable var{ &t1 };

    auto clone = make_blank_variable(&t1);
    auto clone_ptr = clone.get();

    var.set_clone_result(std::move(clone));

    replacements repl;
    auto actual_clone = var.clone_with_replacement(repl);
    MAYFLY_CHECK(actual_clone.get() == clone_ptr);
    MAYFLY_CHECK(repl.variables.at(&var) == clone_ptr);
});

MAYFLY_ADD_TESTCASE("blank variable", [] {
    test_type t1{};

    auto var = make_blank_variable(&t1);
    MAYFLY_REQUIRE(var->get_type() == &t1);

    MAYFLY_REQUIRE(!var->is_constant());
    MAYFLY_REQUIRE(!var->is_member());
    MAYFLY_REQUIRE(!var->is_member_assignment());
});

MAYFLY_END_SUITE;
MAYFLY_END_SUITE;
MAYFLY_END_SUITE;
