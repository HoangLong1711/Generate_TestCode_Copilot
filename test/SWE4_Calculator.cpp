#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "../inc/Calculator.hpp"
#include "../inc/Math.hpp"
#include <limits>

/**
 * ============================================================================
 * SWE4_Calculator.cpp - Optimized Google Test Suite
 * ============================================================================
 * 
 * Module: Calculator
 * Function Under Test: Calculator::addThree(int a, int b, int c)
 * 
 * Function Type: TYPE A (Linear - No Branches, No Conditions) per §16.1
 * Coverage Requirements:
 * - C0 (Statement Coverage): 100% - Single statement executed
 * - C1 (Branch Coverage): N/A - No branches in this function
 * - C2 (Condition Coverage): N/A - No conditions in this function
 * 
 * Test Strategy (§16.1):
 * - MINIMUM 3 tests for complete C0 coverage of linear function
 * - Use TEST_P for consolidation: 1 test method, 3 parameter variants
 * - No redundant property-based tests (commutative, associative, etc.)
 * - Only coverage-driven tests that exercise distinct code paths
 */

// ============================================================================
// Test Fixture Class
// ============================================================================

class CalculatorAddThreeTest : public ::testing::TestWithParam<std::tuple<int, int, int, int>>
{
protected:
    Calculator sut;  // System Under Test
};

// ============================================================================
// Consolidated Test Suite: Minimal Coverage (3 tests)
// ============================================================================

/// ===========================================================================
/// Verifies: Calculator::addThree(int a, int b, int c)
/// Test goal: Validates correct three-parameter addition via math.addTwo()
/// In case: Create Calculator, call addThree(a, b, c), verify result = a + (b + c)
/// Method for Verification: Control flow analysis (C0/C1/C2 coverage)
/// ===========================================================================
INSTANTIATE_TEST_SUITE_P(
    SWE4_Calculator_Minimal_Coverage,
    CalculatorAddThreeTest,
    ::testing::Values(
        // Test 1: Normal case - Execute statement (C0 coverage)
        std::make_tuple(10, 5, 3, 18),
        
        // Test 2: Boundary - Zero values (verify identity behavior)
        std::make_tuple(0, 0, 0, 0),
        
        // Test 3: Boundary - Large values (verify scale independence)
        std::make_tuple(INT_MAX / 4, INT_MAX / 4, INT_MAX / 4, (INT_MAX / 4) * 3)
    )
);

TEST_P(CalculatorAddThreeTest, SWE4_Calculator_addThree_Minimal_Coverage)
{
    auto [a, b, c, expected] = GetParam();
    int actual = sut.addThree(a, b, c);
    EXPECT_EQ(actual, expected);
}

// ============================================================================
// End of Test Suite
// ============================================================================

// Summary:
// - 1 TEST_P method with 3 parameter sets
// - Total: 3 tests for 100% C0 coverage
// - No redundant property-based tests (not needed per §16.2)
// - No unnecessary boundary variants (covered by single large value test)
// - Follows minimization strategy from Testing_Rules.md §16.4
