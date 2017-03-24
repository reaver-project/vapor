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

#include "../helpers.h"

#include "vapor/analyzer/statements/declaration.h"
#include "vapor/analyzer/variables/integer.h"
#include "vapor/parser/declaration.h"

using namespace reaver::vapor;
using namespace reaver::vapor::analyzer;

MAYFLY_BEGIN_SUITE("analyzer");
MAYFLY_BEGIN_SUITE("statements");
MAYFLY_BEGIN_SUITE("declaration");

MAYFLY_ADD_TESTCASE("non-member, explicit type, initializer", [] {
    reaver::default_executor(reaver::make_executor<trivial_executor>());

    {
        scope s1;
        auto current_scope = &s1;

        auto ast = parse(U"let foo : int = 1;", [](auto && arg) { return parser::parse_declaration(arg, parser::declaration_mode::variable_declaration); });

        auto decl = preanalyze_declaration(ast, current_scope);

        MAYFLY_CHECK(current_scope == &s1);
        auto symbol_future = current_scope->get_future(U"foo");
        MAYFLY_CHECK(!symbol_future.try_get());
        current_scope->close();
        MAYFLY_CHECK(symbol_future.try_get() == decl->declared_symbol());

        analysis_context ctx{};
        auto analysis_future = decl->analyze(ctx);
        MAYFLY_CHECK(analysis_future.try_get());

        MAYFLY_CHECK(decl->declared_symbol()->get_variable()->get_type() == builtin_types().integer.get());
        MAYFLY_CHECK(decl->initializer_expression().get()->get_type() == builtin_types().integer.get());

        auto expected_value = integer_constant(1);
        MAYFLY_CHECK(decl->initializer_expression().get()->get_variable()->is_equal(&expected_value));
    }

    {
        scope s1;
        auto s2 = s1.clone_local();
        auto current_scope = s2.get();

        auto ast = parse(U"let foo : int = 1;", [](auto && arg) { return parser::parse_declaration(arg, parser::declaration_mode::variable_declaration); });

        auto decl = preanalyze_declaration(ast, current_scope);

        MAYFLY_CHECK(current_scope != s2.get());
        auto symbol_future = current_scope->get_future(U"foo");
        MAYFLY_CHECK(symbol_future.try_get());
        MAYFLY_CHECK(symbol_future.try_get() == decl->declared_symbol());

        s2.reset(current_scope);
    }
});

MAYFLY_ADD_TESTCASE("non-member, deduced type, initializer", [] {
    reaver::default_executor(reaver::make_executor<trivial_executor>());

    {
        scope s1;
        auto current_scope = &s1;

        auto ast = parse(U"let foo = 1;", [](auto && arg) { return parser::parse_declaration(arg, parser::declaration_mode::variable_declaration); });

        auto decl = preanalyze_declaration(ast, current_scope);

        analysis_context ctx{};
        auto analysis_future = decl->analyze(ctx);
        MAYFLY_CHECK(analysis_future.try_get());

        MAYFLY_CHECK(decl->declared_symbol()->get_variable()->get_type() == builtin_types().integer.get());
        MAYFLY_CHECK(decl->initializer_expression().get()->get_type() == builtin_types().integer.get());

        auto expected_value = integer_constant(1);
        MAYFLY_CHECK(decl->initializer_expression().get()->get_variable()->is_equal(&expected_value));
    }
});

MAYFLY_ADD_TESTCASE("member, deduced type, initializer", [] {
    reaver::default_executor(reaver::make_executor<trivial_executor>());

    {
        scope s1;
        auto current_scope = &s1;

        auto ast = parse(U"let foo = 1;", [](auto && arg) { return parser::parse_declaration(arg, parser::declaration_mode::member_declaration); });

        auto decl = preanalyze_member_declaration(ast, current_scope);

        analysis_context ctx{};
        auto analysis_future = decl->analyze(ctx);
        MAYFLY_CHECK(analysis_future.try_get());

        MAYFLY_CHECK(decl->declared_symbol()->get_variable()->get_type() == builtin_types().integer.get());
        MAYFLY_CHECK(decl->initializer_expression().get()->get_type() == builtin_types().integer.get());

        auto expected_value = integer_constant(1);
        MAYFLY_CHECK(decl->initializer_expression().get()->get_variable()->is_equal(&expected_value));
        MAYFLY_CHECK(decl->declared_symbol()->get_variable()->is_member());
        MAYFLY_CHECK(decl->declared_symbol()->get_variable()->get_default_value()->get_variable()->is_equal(&expected_value));
    }
});

MAYFLY_ADD_TESTCASE("member, explicit type, no initializer", [] {
    reaver::default_executor(reaver::make_executor<trivial_executor>());

    {
        scope s1;
        auto current_scope = &s1;

        auto ast = parse(U"let foo : int;", [](auto && arg) { return parser::parse_declaration(arg, parser::declaration_mode::member_declaration); });

        auto decl = preanalyze_member_declaration(ast, current_scope);

        analysis_context ctx{};
        auto analysis_future = decl->analyze(ctx);
        MAYFLY_CHECK(analysis_future.try_get());

        MAYFLY_CHECK(decl->declared_symbol()->get_variable()->get_type() == builtin_types().integer.get());
        MAYFLY_CHECK(decl->declared_symbol()->get_variable()->is_member());
    }
});

MAYFLY_END_SUITE;
MAYFLY_END_SUITE;
MAYFLY_END_SUITE;
