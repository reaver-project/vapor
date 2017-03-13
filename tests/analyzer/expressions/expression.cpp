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
MAYFLY_BEGIN_SUITE("expressions");
MAYFLY_BEGIN_SUITE("expression");

MAYFLY_ADD_TESTCASE("analysis", [] {
    reaver::default_executor(reaver::make_executor<trivial_executor>());

    test_type t1{};
    test_expression expr{};

    auto var = make_blank_variable(&t1);
    auto var_ptr = var.get();

    expr.set_analysis_variable(std::move(var));

    analysis_context ctx;
    auto analysis_future = expr.analyze(ctx);
    MAYFLY_REQUIRE_NOTHROW(analysis_future.try_get());

    auto reanalysis_future = expr.analyze(ctx);
    MAYFLY_REQUIRE_NOTHROW(reanalysis_future.try_get());

    MAYFLY_CHECK(expr.get_variable() == var_ptr);
    MAYFLY_CHECK(expr.get_type() == &t1);
});

MAYFLY_ADD_TESTCASE("clone cache", [] {
    test_type t1{};
    test_expression expr{};

    auto clone = std::make_unique<test_expression>();
    auto clone_ptr = clone.get();

    expr.set_clone_result(std::move(clone));

    replacements repl;
    auto actual_clone = expr.clone_expr_with_replacement(repl);
    MAYFLY_CHECK(actual_clone.get() == clone_ptr);
    MAYFLY_CHECK(repl.expressions.at(&expr) == clone_ptr);
    MAYFLY_CHECK(repl.statements.at(&expr) == clone_ptr);
});

MAYFLY_ADD_TESTCASE("simplification", [] {
    reaver::default_executor(reaver::make_executor<trivial_executor>());

    test_expression expr{};

    auto simplified = std::make_unique<test_expression>();
    auto simplified_ptr = simplified.get();

    expr.set_simplified_expression(std::move(simplified));

    simplification_context ctx;
    auto simpl_future = expr.simplify_expr(ctx);
    MAYFLY_CHECK(simpl_future.try_get() == simplified_ptr);

    auto resimpl_future = expr.simplify_expr(ctx);
    MAYFLY_CHECK(resimpl_future.try_get() == simplified_ptr);

    auto stmt_simpl_future = expr.simplify(ctx);
    MAYFLY_CHECK(stmt_simpl_future.try_get() == simplified_ptr);

    auto ctx_future = ctx.get_future_or_init<expression>(&expr, []() -> reaver::future<expression *> {
        MAYFLY_REQUIRE(!"the context doesn't hold the required future!");
        __builtin_unreachable();
    });
    MAYFLY_CHECK(ctx_future.try_get() == simplified_ptr);

    delete simplified_ptr;

    simplification_context other_ctx;
    auto bad_simpl_future = expr.simplify_expr(other_ctx);
    MAYFLY_CHECK_THROWS_TYPE(unexpected_call, bad_simpl_future.try_get());
    MAYFLY_CHECK_THROWS_TYPE(unexpected_call, expr.simplify(other_ctx).try_get());
});

MAYFLY_END_SUITE;
MAYFLY_END_SUITE;
MAYFLY_END_SUITE;
