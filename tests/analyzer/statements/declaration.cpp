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

#include <reaver/future_get.h>

#include "vapor/analyzer/expressions/integer.h"
#include "vapor/analyzer/statements/declaration.h"
#include "vapor/parser/declaration.h"

using namespace reaver::vapor;
using namespace reaver::vapor::analyzer;

MAYFLY_BEGIN_SUITE("analyzer");
MAYFLY_BEGIN_SUITE("statements");
MAYFLY_BEGIN_SUITE("declaration");

MAYFLY_ADD_TESTCASE("non-member, explicit type, initializer", [] {
    {
        scope s1;
        auto current_scope = &s1;

        auto ast = parse(U"let foo : int = 1;", [](auto && arg) { return parser::parse_declaration(arg, parser::declaration_mode::variable_declaration); });

        auto decl = preanalyze_declaration(ast, current_scope);

        MAYFLY_CHECK(current_scope == &s1);
        auto symbol_future = current_scope->get_future(U"foo");
        MAYFLY_CHECK(!symbol_future.try_get());
        current_scope->close();
        MAYFLY_CHECK(reaver::get(symbol_future) == decl->declared_symbol());

        analysis_context ctx{};
        auto analysis_future = decl->analyze(ctx);
        reaver::get(analysis_future);

        MAYFLY_CHECK(decl->declared_symbol()->get_expression()->get_type() == builtin_types().integer.get());
        MAYFLY_CHECK(decl->initializer_expression().get()->get_type() == builtin_types().integer.get());

        integer_constant expected_value{ 1 };
        MAYFLY_CHECK(decl->initializer_expression().get()->is_equal(&expected_value));

        auto init_expr = decl->initializer_expression().get();
        simplification_context simpl_ctx{};
        auto simpl_future = decl->simplify({ simpl_ctx });
        std::unique_ptr<statement> simplified{ reaver::get(simpl_future) };
        MAYFLY_CHECK(simplified.get() == init_expr);
    }

    {
        scope s1;
        auto s2 = s1.clone_local();
        auto current_scope = s2.get();

        auto ast = parse(U"let foo : int = 1;", [](auto && arg) { return parser::parse_declaration(arg, parser::declaration_mode::variable_declaration); });

        auto decl = preanalyze_declaration(ast, current_scope);

        MAYFLY_CHECK(current_scope != s2.get());
        auto symbol_future = current_scope->get_future(U"foo");
        MAYFLY_CHECK(reaver::get(symbol_future) == decl->declared_symbol());

        s2.reset(current_scope);
    }
});

MAYFLY_ADD_TESTCASE("non-member, deduced type, initializer", [] {
    {
        scope s1;
        auto current_scope = &s1;

        auto ast = parse(U"let foo = 1;", [](auto && arg) { return parser::parse_declaration(arg, parser::declaration_mode::variable_declaration); });

        auto decl = preanalyze_declaration(ast, current_scope);

        analysis_context ctx{};
        auto analysis_future = decl->analyze(ctx);
        reaver::get(analysis_future);

        MAYFLY_CHECK(decl->declared_symbol()->get_expression()->get_type() == builtin_types().integer.get());
        MAYFLY_CHECK(decl->initializer_expression().get()->get_type() == builtin_types().integer.get());

        integer_constant expected_value{ 1 };
        MAYFLY_CHECK(decl->initializer_expression().get()->is_equal(&expected_value));
    }
});

MAYFLY_ADD_TESTCASE("member, deduced type, initializer", [] {
    {
        scope s1;
        auto current_scope = &s1;

        auto ast = parse(U"let foo = 1;", [](auto && arg) { return parser::parse_declaration(arg, parser::declaration_mode::member_declaration); });

        auto decl = preanalyze_member_declaration(ast, current_scope);

        analysis_context ctx{};
        auto analysis_future = decl->analyze(ctx);
        reaver::get(analysis_future);

        MAYFLY_CHECK(decl->declared_symbol()->get_expression()->get_type() == builtin_types().integer.get());
        MAYFLY_CHECK(decl->initializer_expression().get()->get_type() == builtin_types().integer.get());

        integer_constant expected_value{ 1 };
        MAYFLY_CHECK(decl->initializer_expression().get()->is_equal(&expected_value));
        MAYFLY_CHECK(decl->declared_symbol()->get_expression()->is_member());
        MAYFLY_CHECK(decl->declared_symbol()->get_expression()->get_default_value()->is_equal(&expected_value));
    }
});

MAYFLY_ADD_TESTCASE("member, explicit type, no initializer", [] {
    {
        scope s1;
        auto current_scope = &s1;

        auto ast = parse(U"let foo : int;", [](auto && arg) { return parser::parse_declaration(arg, parser::declaration_mode::member_declaration); });

        auto decl = preanalyze_member_declaration(ast, current_scope);

        analysis_context ctx{};
        auto analysis_future = decl->analyze(ctx);
        reaver::get(analysis_future);

        MAYFLY_CHECK(decl->declared_symbol()->get_expression()->get_type() == builtin_types().integer.get());
        MAYFLY_CHECK(decl->declared_symbol()->get_expression()->is_member());
    }
});

MAYFLY_END_SUITE;
MAYFLY_END_SUITE;
MAYFLY_END_SUITE;
