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

#include <reaver/future_get.h>

#include "../helpers.h"

using namespace reaver::vapor;
using namespace reaver::vapor::analyzer;

MAYFLY_BEGIN_SUITE("analyzer");
MAYFLY_BEGIN_SUITE("expressions");
MAYFLY_BEGIN_SUITE("expression");

MAYFLY_ADD_TESTCASE("analysis", [] {
    test_type t1{};
    test_expression expr{};

    expr.set_analysis_type(&t1);

    analysis_context ctx;
    auto analysis_future = expr.analyze(ctx);
    MAYFLY_REQUIRE_NOTHROW(reaver::get(analysis_future));

    auto reanalysis_future = expr.analyze(ctx);
    MAYFLY_REQUIRE_NOTHROW(reaver::get(reanalysis_future));

    MAYFLY_CHECK(expr.get_type() == &t1);
});

MAYFLY_ADD_TESTCASE("clone cache", [] {
    test_type t1{};
    test_expression expr{};

    auto clone = std::make_unique<test_expression>();
    auto clone_ptr = clone.get();

    expr.set_clone_result(std::move(clone));

    replacements repl;
    auto actual_clone = repl.claim(&expr);
    MAYFLY_CHECK(actual_clone.get() == clone_ptr);
    MAYFLY_CHECK(repl.get_replacement(&expr) == clone_ptr);
    MAYFLY_CHECK(repl.get_replacement(static_cast<statement *>(&expr)) == clone_ptr);
});

MAYFLY_ADD_TESTCASE("simplification", [] {
    MAYFLY_MAIN_THREAD;

    test_expression expr{};

    auto simplified = std::make_unique<test_expression>();
    auto simplified_ptr = simplified.get();

    expr.set_simplified_expression(std::move(simplified));

    cached_results res;
    simplification_context ctx{ res };
    auto simpl_future = expr.simplify_expr({ ctx });
    MAYFLY_REQUIRE(reaver::get(simpl_future) == simplified_ptr);

    auto resimpl_future = expr.simplify_expr({ ctx });
    MAYFLY_REQUIRE(reaver::get(resimpl_future) == simplified_ptr);

    auto stmt_simpl_future = expr.simplify({ ctx });
    MAYFLY_REQUIRE(reaver::get(stmt_simpl_future) == simplified_ptr);

    auto ctx_future = ctx.get_future_or_init<expression>(&expr, [&]() -> reaver::future<expression *> {
        MAYFLY_THREAD;
        MAYFLY_REQUIRE(!"the context doesn't hold the required future!");
        __builtin_unreachable();
    });
    MAYFLY_REQUIRE(reaver::get(ctx_future) == simplified_ptr);

    delete simplified_ptr;

    simplification_context other_ctx{ res };
    auto bad_simpl_future = expr.simplify_expr({ other_ctx });
    MAYFLY_CHECK_THROWS_TYPE(unexpected_call, reaver::get(bad_simpl_future));
    MAYFLY_CHECK_THROWS_TYPE(unexpected_call, reaver::get(expr.simplify({ other_ctx })));
});

MAYFLY_END_SUITE;
MAYFLY_END_SUITE;
MAYFLY_END_SUITE;
