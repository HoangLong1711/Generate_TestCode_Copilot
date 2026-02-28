# Testing Rules for SWE4 Unit Test Generation

This document defines the comprehensive rules and standards for generating unit test cases following ISO 26262 ASIL D compliance and Google Test/GoogleMock framework conventions.

---

## ⚠️ CRITICAL RULE #1: Production Code Modification FORBIDDEN

### Absolute Rule: NEVER Modify src/ or inc/ Directories

**Under NO circumstances shall production code be modified during test development. This is the most important rule.**

### What This Means

- ❌ **DO NOT** add `friend` declarations to classes in inc/*.hpp
- ❌ **DO NOT** add public getter/setter functions just for testing
- ❌ **DO NOT** add test-only conditional code in src/*.cpp
- ❌ **DO NOT** modify constructors, destructors, or method signatures
- ❌ **DO NOT** add debug or logging code specifically for tests
- ❌ **DO NOT** change existing public interfaces
- ❌ **DO NOT** add any code whatsoever to src/ or inc/ folders

### What You MUST Do Instead

- ✅ **Work with existing public interface** — Test only what is already public
- ✅ **If getter needed**: It must have been designed as part of production API
- ✅ **If private state must be tested**: Use existing public control functions (see §10.2)
- ✅ **Refactor production code separately** — Make separate commit/PR if public API needs expansion
- ✅ **Test through side effects** — Observe public member changes to infer private state

### Rationale: Why This Rule Is MOST CRITICAL

1. **Production Code Integrity**: Ensures tests validate actual production behavior, not mock behavior added for tests
2. **ISO 26262 Compliance**: Functional safety requires testing real code paths, not test-specific variations
3. **Code Maintainability**: Avoids technical debt of test-specific code littering production
4. **Architecture Respect**: Maintains original design decisions about encapsulation and public API
5. **Traceability**: Test-production separation ensures clear audit trails for safety-critical systems
6. **Audit Trail**: For compliance purposes, production code must be unmodified by test generation

---
**STRICT TEST VALUE RULE:**

Tests that vary numeric input values must only be created when a specific system requirement or the SDD explicitly mandates those values. The test generator must be able to point to the exact requirement/SDD clause that justifies each numeric test case. Generating numeric test cases solely to explore value combinations, mathematical properties, or to chase coverage without SDD traceability is forbidden.


---

## 1. Test File Structure and Naming

### Test File Naming Convention
- **Format**: `SWE4_<ModuleName>.cpp`
- **Module Name**: Extracted from source file name (Calculator.cpp → Calculator)
- **Location**: `test/` folder
- **Example**: `test/SWE4_Calculator.cpp`

### Test Class Naming Convention
- **Format**: `<ModuleName>UnitTest : public ::testing::Test`
- **Inheritance**: All test classes inherit from `::testing::Test`
- **Scope**: One test class per module file
- **Example**: `class CalculatorUnitTest : public ::testing::Test`

### Test Method Naming Convention
- **Format**: `SWE4_<ModuleName>_<FunctionName>_<Scenario>`
- **Scenario Types**: 
  - `_Normal_<Description>` — Standard operation with typical inputs
  - `_Boundary_<Description>` — Edge values (0, min, max, equality boundaries)
  - `_Error_<Description>` — Invalid inputs, negative numbers, exception handling
- **Examples**:
  - `SWE4_Calculator_complexAdd_Normal_Positive_Sum_Less_100`
  - `SWE4_Calculator_complexAdd_Boundary_Zero_Input`
  - `SWE4_Calculator_complexAdd_Error_Negative_Sum`

---

## 2. Test Case Documentation Header (Mandatory)

Each test case MUST include a documentation header following this template:

```cpp
/// ===========================================================================
/// Verifies: <Qualified Function Name>
/// Test goal: <One-line description of what is being tested>
/// In case: <Step-by-step setup and trigger>
/// Method for Verification: Control flow analysis (C0/C1/C2 coverage)
/// ===========================================================================
TEST_F(CalculatorUnitTest, SWE4_Calculator_complexAdd_Normal_Positive_Sum_Less_100) {
    // ... test implementation
}
```

### Header Field Definitions

| Field | Description | Example |
|-------|-------------|---------|
| **Verifies** | Fully qualified function name being tested | `Calculator::complexAdd()` |
| **Test goal** | Precise description of what this test validates | `Validates correct summation when sum < 100` |
| **In case** | Step-by-step setup: Create object → Set inputs → Execute → Check result | `Create Calculator instance, call complexAdd(10, 20), verify result equals 30` |
| **Method** | Coverage method (always "Control flow analysis (C0/C1/C2 coverage)") | Fixed text |

---

## 3. Test Setup and Teardown (SetUp/TearDown)

### SetUp Method (Optional, if needed for shared test initialization)
```cpp
void SetUp() override {
    // Initialize common test fixtures
    sut = new Calculator();  // SUT = System Under Test
}
```

### TearDown Method (Optional, for cleanup)
```cpp
void TearDown() override {
    // Clean up resources
    delete sut;
}
```

### Naming Convention
- Use `sut` for the System Under Test object
- Use `actual` for the value returned by the function under test
- Use `expected` for the expected result value

---

## 4. Test Coverage Requirements (C0/C1/C2)

### C0 - Statement Coverage
- **Definition**: Every executable statement must be executed at least once
- **Requirement**: 100% of all statements in production code must be covered
- **Validation**: Track which lines are executed in test execution

### C1 - Branch Coverage
- **Definition**: Every branch (true/false) of conditional statements must be executed
- **Requirement**: Both if/else paths, all switch cases, loop iterations
- **Example**: For `if (a > b) {...} else {...}` — test both conditions as true and false
- **Validation**: Verify both decision branches are taken

### C2 - Condition Coverage
- **Definition**: All combinations of condition operands must be tested
- **Requirement**: For complex conditions like `if (a > 0 && b < 10)`, test all combinations:
  - `a > 0` TRUE, `b < 10` TRUE
  - `a > 0` TRUE, `b < 10` FALSE
  - `a > 0` FALSE, `b < 10` TRUE
  - `a > 0` FALSE, `b < 10` FALSE
- **Validation**: mcdc (Modified Condition/Decision Coverage) analysis

### Target Coverage Level
- **Minimum**: 100% C0, 100% C1, 100% C2
- **Standard**: ISO 26262 ASIL D functional safety compliance
- **Measurement**: Using gcovr tool to analyze .gcda coverage data

---

## 5. Test Scope: Normal, Boundary, Error Cases

For each function, generate test cases across three categories:

### 5.1 Normal Cases
- **Purpose**: Verify standard operation with typical inputs
- **Characteristics**:
  - Representative positive/negative values
  - Values that exercise main code paths
  - Examples: small integers, common calculations, standard object states
 - **Rule**: Only add normal-case value variations when they are explicitly required by the system requirements or the SDD. Do NOT add exploratory or property-based numeric tests that are not traced to a requirement.
 - **When required**: If the requirement/SDD demands specific input values, include the minimal set required to demonstrate the behaviour for those values (typically 1–2 representative values). Otherwise, omit extra normal numeric variants and rely on the boundary suite to drive coverage.

### 5.2 Boundary Cases
- **Purpose**: Verify correct behavior at edge values
- **Characteristics**:
  - Zero boundary (0, -0, near-zero)
  - Min/Max values (INT_MIN, INT_MAX, 0, -1, 1)
  - Equality boundaries (a==b, a>b, a<b)
  - Off-by-one values (n-1, n, n+1)
  - Loop iteration boundaries (0 iterations, 1 iteration, many iterations)
- **Minimum**: 3-5 test cases per function

### 5.3 Error Cases
- **Purpose**: Verify graceful handling of invalid inputs
- **Characteristics**:
  - Negative numbers (if applicable)
  - Out-of-range values
  - Invalid object states
  - Exception scenarios (if applicable)
  - Overflow/underflow conditions
- **Minimum**: 2-3 test cases per function

---

## 6. Mocking Strategy

### 6.1 What to Mock
Mock **external dependencies** to isolate the unit under test:
- External library functions (e.g., `mt::Math` namespace functions)
- Third-party class methods
- Global singleton instances
- Hardware I/O or network calls

### 6.2 Mock Implementation Pattern

#### Create a Mock Stub Class
```cpp
class MockMath {
public:
    MOCK_METHOD(int, add, (int a, int b), (const));
    MOCK_METHOD(int, multiply, (int a, int b), (const));
};
```

#### Use NiceMock Pattern
```cpp
class CalculatorUnitTest : public ::testing::Test {
protected:
    NiceMock<MockMath> mockMath;  // Suppress warnings for uninteresting calls
    Calculator sut;
};
```

#### Set Expectations with EXPECT_CALL
```cpp
TEST_F(CalculatorUnitTest, SWE4_Calculator_complexAdd_Normal_Calls_Math) {
    EXPECT_CALL(mockMath, add(10, 20))
        .WillOnce(::testing::Return(30));
    
    int actual = sut.complexAdd(10, 20);
    EXPECT_EQ(actual, 30);
}
```

### 6.3 What NOT to Mock
**Do NOT mock** internal dependencies:
- Functions in the same class
- Internal helper functions
- Private methods
- Class member objects (use real instances)

### 6.4 Mock Matchers (For Flexible Assertions)
Common matchers for EXPECT_CALL:
- `::testing::Eq(value)` — Exact equality
- `::testing::Ge(value)` — Greater than or equal
- `::testing::Le(value)` — Less than or equal
- `::testing::Gt(value)` — Greater than
- `::testing::Lt(value)` — Less than
- `::testing::Ne(value)` — Not equal
- `::testing::_` — Any value (wildcard)
- `::testing::AllOf(m1, m2, ...)` — All matchers must pass

---

## 7. Test Method Implementation Pattern

### CRITICAL RULE: Test Macros ALLOWED
**Only `TEST_F` and `TEST_P` macros are permitted for test implementation.**
- ✅ **ALLOWED**: `TEST_F(FixtureName, TestName)` — Fixture-based tests
- ✅ **ALLOWED**: `TEST_P(ParameterizedFixture, TestName)` — Parameterized tests
- ❌ **FORBIDDEN**: `TEST()` — Plain tests without fixtures
- ❌ **FORBIDDEN**: Custom test macros or non-standard patterns

### 7.1 TEST_F Pattern (Fixture-Based Tests)

```cpp
TEST_F(CalculatorUnitTest, SWE4_Calculator_complexAdd_Normal_Positive_Sum_Less_100) {
    /// ===========================================================================
    /// Verifies: Calculator::complexAdd()
    /// Test goal: Validates correct summation when sum < 100
    /// In case: Create Calculator, call complexAdd(10, 20), verify result equals 30
    /// Method for Verification: Control flow analysis (C0/C1/C2 coverage)
    /// ===========================================================================
    
    // Arrange: Set up test conditions and mocks
    Calculator sut;
    int a = 10, b = 20;
    int expected = 30;
    
    // Act: Execute the function under test
    int actual = sut.complexAdd(a, b);
    
    // Assert: Verify the result
    EXPECT_EQ(actual, expected);
}
```

**When to Use TEST_F**:
- Complex test scenarios requiring unique setup or conditional logic
- Tests with distinct verification requirements (different assertions)
- Tests requiring mock object initialization (via SetUp())
- Scenarios with special preconditions or state management
- Error/exception handling tests
- Tests that cannot be parameterized (non-numeric variations)

### 7.2 TEST_P Pattern (Parameterized Boundary Value Analysis)

```cpp
class CalculatorParameterizedTest : public ::testing::TestWithParam<std::pair<int, int>> {
protected:
    Calculator sut;
};

// Test cases: Parameter pairs where (input, expected_output) or (a, b) remain constant
// but numeric values vary across different magnitudes
INSTANTIATE_TEST_SUITE_P(
    SWE4_Calculator_Normal_Cases,
    CalculatorParameterizedTest,
    ::testing::Values(
        std::make_pair(5, 3),      // Normal: small positive
        std::make_pair(100, 200),  // Normal: large positive
        std::make_pair(-5, -3),    // Normal: negative
        std::make_pair(1, 1)       // Normal: minimal values
    )
);

TEST_P(CalculatorParameterizedTest, SWE4_Calculator_complexAdd_Normal_Multiple_Values) {
    /// ===========================================================================
    /// Verifies: Calculator::complexAdd()
    /// Test goal: Validates correct addition across multiple representative values
    /// In case: Create Calculator, call complexAdd(a, b), verify result equals a+b
    /// Method for Verification: Control flow analysis (C0/C1/C2 coverage)
    /// ===========================================================================
    
    auto [a, b] = GetParam();
    int expected = a + b;
    int actual = sut.complexAdd(a, b);
    EXPECT_EQ(actual, expected);
}

// CRITICAL: Boundary and Overflow Testing - MANDATORY for all numeric functions
class CalculatorBoundaryTest : public ::testing::TestWithParam<std::pair<int, int>> {
protected:
    Calculator sut;
};

INSTANTIATE_TEST_SUITE_P(
    SWE4_Calculator_Boundary_Values,
    CalculatorBoundaryTest,
    ::testing::Values(
        std::make_pair(0, 0),                      // Zero boundary
        std::make_pair(INT_MAX, 0),                // Maximum value
        std::make_pair(INT_MIN, 0),                // Minimum value
        std::make_pair(INT_MAX / 2, INT_MAX / 2),  // Mid-range positive
        std::make_pair(INT_MIN / 2, INT_MIN / 2),  // Mid-range negative
        std::make_pair(INT_MAX - 1, 1)             // Optional near-boundary (INT_MAX-1 / INT_MIN+1)
    )
);

TEST_P(CalculatorBoundaryTest, SWE4_Calculator_complexAdd_Boundary_Edge_Values) {
    /// ===========================================================================
    /// Verifies: Calculator::complexAdd()
    /// Test goal: Validates correct behavior at boundary and edge values
    /// In case: Create Calculator, test with INT_MAX/INT_MIN/zero values
    /// Method for Verification: Control flow analysis (C0/C1/C2 coverage)
    /// ===========================================================================
    
    auto [a, b] = GetParam();
    int expected = a + b;
    int actual = sut.complexAdd(a, b);
    EXPECT_EQ(actual, expected);
}
```

**CRITICAL RULE: Use TEST_P When**:
- ✅ **SAME function, SAME logic, DIFFERENT numeric values** — Use parameterized test
- ✅ Boundary value testing using the STRICT minimal set (see below)
- ✅ Reduces code duplication for similar test cases
- ✅ Numeric range analysis testing

**MANDATORY: Overflow/Underflow Testing (STRICT)**:
For all functions with numeric inputs, include a minimal, focused boundary suite consisting of these cases (total ≈ 5–6 tests):
1. `0` (zero)
2. `INT_MAX` (maximum)
3. `INT_MIN` (minimum)
4. `INT_MAX/2` (mid positive)
5. `INT_MIN/2` (mid negative)
6. Optional: one near-boundary case (e.g., `INT_MAX-1` or `INT_MIN+1`) when a near-limit behavior must be validated

Notes:
- Aim for approximately 5 tests for boundaries; up to 6 if the optional near-boundary test is required.
- Do not add additional redundant numeric variants that only change magnitudes without exercising new branches or decisions.
- Document expected behavior at limits (wrap, saturate, undefined, etc.) in the test header.
- Mandatory traceability: every numeric test case MUST be linked to a specific requirement or SDD clause. Do NOT add numeric test cases solely for exploratory coverage or mathematical properties — only add a value test when the requirement/SDD explicitly requires that value or when the value is necessary to exercise a documented behavior.

Example Overflow Test:
```cpp
// Test: What happens when adding near INT_MAX values?
// Expected: Result wraps or saturates (depending on implementation)
INSTANTIATE_TEST_SUITE_P(
    SWE4_Calculator_Overflow_Cases,
    CalculatorOverflowTest,
    ::testing::Values(
        std::make_pair(INT_MAX - 100, 50),    // Exceeds INT_MAX
        std::make_pair(INT_MIN + 100, -50),   // Exceeds INT_MIN
        std::make_pair(INT_MAX / 2, INT_MAX / 2)  // Boundary approach
    )
);

TEST_P(CalculatorOverflowTest, SWE4_Calculator_complexAdd_Overflow_Behavior) {
    auto [a, b] = GetParam();
    // Note: Document expected behavior (wrap, saturate, undefined, etc.)
    int actual = sut.complexAdd(a, b);
    // Verify documented behavior (may differ per implementation)
    EXPECT_EQ(actual, a + b);  // Or alternative documented behavior
}
```

### 7.3 Assertion Pattern
- **Primary Assertions**: `EXPECT_EQ()`, `EXPECT_NE()`, `EXPECT_LT()`, `EXPECT_LE()`, `EXPECT_GT()`, `EXPECT_GE()`
- **Boolean Assertions**: `EXPECT_TRUE()`, `EXPECT_FALSE()`
- **Floating Point**: `EXPECT_FLOAT_EQ()`, `EXPECT_DOUBLE_EQ()`
- **String Assertions**: `EXPECT_STREQ()`, `EXPECT_STRNE()`
- **Pointer Assertions**: `EXPECT_NE(ptr, nullptr)`, `EXPECT_EQ(ptr, nullptr)`

---

## 8. Includes and Headers

### Required Headers for Every Test File
```cpp
#include <gtest/gtest.h>
#include <gmock/gmock.h>

// Production headers (under test)
#include "../inc/Calculator.hpp"
#include "../inc/Math.hpp"

// Standard library headers (if needed)
#include <stdexcept>
#include <limits>
```

### Header Organization
1. Google Test/Mock headers (gtest, gmock)
2. Production code headers (relative includes to inc/)
3. Standard C++ library headers

---

## 9. Test Data and Constants

### Use Named Constants for Test Values
```cpp
class CalculatorUnitTest : public ::testing::Test {
protected:
    static constexpr int POSITIVE_VALUE = 10;
    static constexpr int NEGATIVE_VALUE = -5;
    static constexpr int BOUNDARY_ZERO = 0;
    static constexpr int LARGE_VALUE = 1000;
};
```

### Avoid Magic Numbers
- ❌ **Bad**: `int actual = sut.complexAdd(42, 58);`
- ✅ **Good**: `int a = 42; int b = 58; int actual = sut.complexAdd(a, b);`

---

## 10. Testing Private Functions and Variables

### CRITICAL RULE: Testing Private Members Through Public Interface ONLY

**Principle**: All testing of private functions and variables MUST occur through public member functions. Never use friend declarations or direct access.

### 10.1 Private Functions

**Rule**: Private functions are tested indirectly through public functions that call them.

```cpp
class Calculator {
private:
    int internalAdd(int a, int b) {  // Private helper
        return a + b;
    }
public:
    int complexAdd(int a, int b, int c) {  // Public - calls private internalAdd
        return internalAdd(a, b) + c;
    }
};

// TEST: internalAdd() is tested via complexAdd()
TEST_F(CalculatorUnitTest, SWE4_Calculator_complexAdd_Tests_internalAdd) {
    Calculator sut;
    int actual = sut.complexAdd(10, 20, 5);  // Exercises internalAdd(10, 20)
    EXPECT_EQ(actual, 35);  // 10 + 20 + 5 = 35
}
```

### 10.2 Private Variables

**Rule**: Private variables are accessed/verified through:
1. **Getter functions** — Public methods that return private member values
2. **Public behavior** — Observable side effects on public members
3. **Control flow analysis** — Code coverage showing private variable state changes

**Strategy: Find Set/Get/Control Functions**
- Search for `void set<VarName>()`, `get<VarName>()`, or `control<VarName>()`
- Use code flow analysis to trace private variable modifications
- Test observable effects on public members when private state changes

```cpp
class Device {
private:
    int state;  // Private
public:
    void setState(int s) { state = s; }
    int getState() const { return state; }
    int processWithState(int value) { return value * state; }
};

// TEST: private 'state' via setter/getter
TEST_F(DeviceUnitTest, SWE4_Device_privateState_Via_Setter_Getter) {
    Device sut;
    sut.setState(5);  // Control private variable via public setter
    EXPECT_EQ(sut.getState(), 5);  // Verify via public getter
}

// TEST: private 'state' affects public behavior
TEST_F(DeviceUnitTest, SWE4_Device_privateState_Affects_Public_Method) {
    Device sut;
    sut.setState(3);
    int actual = sut.processWithState(10);  // Private state affects result
    EXPECT_EQ(actual, 30);  // 10 * 3 = 30
}
```

### 10.3 Anti-Patterns (FORBIDDEN)

- ❌ **Friend Declarations**: `friend class CalculatorUnitTest;` — DO NOT USE
- ❌ **Direct Private Access**: `sut.state = 5;` — DO NOT USE
- ❌ **Casting away access**: Using `const_cast<>` to bypass encapsulation — DO NOT USE
- ❌ **Reflection/Runtime introspection**: Accessing members via member pointers — DO NOT USE

**Correct Approach**:
- ✅ Always use public member functions (getters, setters, control functions)
- ✅ Test private behavior through public interface
- ✅ Trace code flow in documentation to explain private state coverage

### Rules for Test Independence
1. **No Shared State**: Each test must be independent; tests should not rely on results from other tests
2. **SetUp/TearDown**: Use SetUp() and TearDown() for common initialization/cleanup
3. **No Test Ordering**: Tests can run in any order and produce the same results
4. **No File/Network I/O**: Use mocks instead of actual file or network operations
5. **Deterministic Results**: Same input always produces same output (no random data)

### Anti-Patterns
- ❌ Tests depending on execution order
- ❌ Using global variables that persist across tests
- ❌ File system or network calls in test code
- ❌ Timing-dependent assertions
- ❌ Uninitialized variables

---

## 11. Coverage Analysis and Reporting

### Coverage Metrics to Track
- **C0 Coverage**: Percentage of statements executed (target: 100%)
- **C1 Coverage**: Percentage of branches executed (target: 100%)
- **C2 Coverage**: Percentage of condition combinations executed (target: 100%)

### Uncovered Code Documentation
If after 5 iterations coverage remains < 100%, document:
- **Line Number**: Exact source code line(s) not covered
- **Reason**: Why this line is unreachable (e.g., defensive error handling, compiler-specific code, platform-specific code)
- **Path Analysis**: Explain the control flow that cannot reach this code
- **Acceptance**: Formally accept uncovered code as justified dead code or architecture-specific

### Example Documentation
```
## Uncovered Code Analysis

### File: src/Calculator.cpp, Line 45
**Code**: `if (a < INT_MIN + b)  // Overflow check`
**Reason**: The compiler's signed integer arithmetic saturates before overflow; reaching this condition would require undefined behavior
**Path Analysis**: To execute line 45, both `a` and `b` must be negative with `a < INT_MIN + b`. However, this violates C++ signed integer semantics in practice.
**Status**: Justified dead code (defensive programming, theoretically unreachable)
```

---

## 12. Tools and Framework Requirements

### Google Test (gtest)
- **Version**: Latest (auto-fetched by CMake)
- **Headers**: `#include <gtest/gtest.h>`
- **Macros**: `TEST_F()`, `EXPECT_EQ()`, `EXPECT_CALL()`

### Google Mock (gmock)
- **Version**: Bundled with gtest
- **Headers**: `#include <gmock/gmock.h>`
- **Macros**: `MOCK_METHOD()`, `NiceMock<>`, `EXPECT_CALL()`

### Coverage Tool
- **Tool**: gcovr with standard LCOV HTML output format
- **Flags**: `--coverage -fprofile-arcs -ftest-coverage` (in CMakeLists.txt)
- **Report Format**: Standard LCOV HTML (auto-generated by gcovr)
- **Report Location**: `reports/coverage_<module>.html`
- **Report Contents**: 
  - Line coverage with hit counts
  - Branch coverage visualization
  - Function-level metrics
  - Source code line-by-line annotation

### Build System
- **CMake**: Version 3.10+
- **Compiler**: GCC/MinGW (Windows)
- **Standard**: C++17

### Handling Missing Build/Library Dependencies

If the target machine does not have the required build tools or libraries installed, the automated agent MUST NOT attempt to substitute by generating alternate analysis artifacts (for example, by producing standalone JS/HTML/CSS reports and performing its own offline analysis). Instead the agent must:

- **Notify the user** with a clear message that required build tools or libraries are missing and that a proper build environment is required to compile and run the tests.
- **List the missing requirements** (minimum required items) so the user can install them manually. At minimum this list should include:
    - CMake (>= 3.10)
    - A C/C++ compiler toolchain (GCC/MinGW or MSVC)
    - Google Test / Google Mock (gtest, gmock)
    - gcov/gcovr (coverage tooling)
    - Standard development utilities (make, ninja or equivalent)
- **Provide optional platform hints** for package names or install sources (e.g., "gtest/gmock via your distribution's packages or fetched by CMake ExternalProject"), but do not assume or perform installation.
- **Halt further automatic report-generation work** that depends on a successful native build; instead await user action after dependencies are satisfied.

This approach preserves correctness and traceability for safety-critical builds and ensures the user explicitly controls the build environment.


## 13. Example Test File Template

```cpp
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "../inc/Calculator.hpp"
#include "../inc/Math.hpp"
#include <limits>

// ============================================================================
// Mock Classes
// ============================================================================

class MockMath {
public:
    MOCK_METHOD(int, add, (int a, int b), (const));
};

// ============================================================================
// Test Fixture Class
// ============================================================================

class CalculatorUnitTest : public ::testing::Test {
protected:
    static constexpr int POSITIVE_SMALL = 10;
    static constexpr int POSITIVE_LARGE = 90;
    static constexpr int BOUNDARY_ZERO = 0;
    static constexpr int BOUNDARY_MAX = INT_MAX;
    static constexpr int BOUNDARY_MIN = INT_MIN;
    
    void SetUp() override {
        // Initialize test fixtures
    }
    
    void TearDown() override {
        // Clean up resources
    }
};

// ============================================================================
// Test Cases: Normal Operation
// ============================================================================

/// ===========================================================================
/// Verifies: Calculator::complexAdd()
/// Test goal: Validates correct addition with positive small numbers
/// In case: Create Calculator, call complexAdd(10, 20), verify result equals 30
/// Method for Verification: Control flow analysis (C0/C1/C2 coverage)
/// ===========================================================================
TEST_F(CalculatorUnitTest, SWE4_Calculator_complexAdd_Normal_Positive_Small) {
    Calculator sut;
    int actual = sut.complexAdd(POSITIVE_SMALL, POSITIVE_SMALL);
    EXPECT_EQ(actual, 20);
}

// ============================================================================
// Test Cases: Boundary Value Analysis
// ============================================================================

/// ===========================================================================
/// Verifies: Calculator::complexAdd()
/// Test goal: Validates correct behavior when input is zero
/// In case: Create Calculator, call complexAdd(0, 0), verify result equals 0
/// Method for Verification: Control flow analysis (C0/C1/C2 coverage)
/// ===========================================================================
TEST_F(CalculatorUnitTest, SWE4_Calculator_complexAdd_Boundary_Zero_Inputs) {
    Calculator sut;
    int actual = sut.complexAdd(BOUNDARY_ZERO, BOUNDARY_ZERO);
    EXPECT_EQ(actual, 0);
}

// ============================================================================
// Test Cases: Error Handling
// ============================================================================

/// ===========================================================================
/// Verifies: Calculator::complexAdd()
/// Test goal: Validates handling of negative numbers
/// In case: Create Calculator, call complexAdd(-5, -10), verify result equals -15
/// Method for Verification: Control flow analysis (C0/C1/C2 coverage)
/// ===========================================================================
TEST_F(CalculatorUnitTest, SWE4_Calculator_complexAdd_Error_Negative_Values) {
    Calculator sut;
    int actual = sut.complexAdd(-5, -10);
    EXPECT_EQ(actual, -15);
}
```

---

## 14. Checklist for Test Completeness

Before finalizing a test file, verify:

- [ ] **File Naming**: `SWE4_<ModuleName>.cpp` in `test/` folder
- [ ] **Test Class**: Inherits from `::testing::Test`
- [ ] **Test Methods**: Follow `SWE4_<ModuleName>_<Function>_<Scenario>` naming
- [ ] **Headers**: All documentation headers present and complete
- [ ] **Coverage**: Normal, Boundary, Error cases for all functions
- [ ] **Mocking**: External dependencies mocked; internal code tested with real instances
- [ ] **Assertions**: Each test has clear, specific assertions
- [ ] **Independence**: Tests can run in any order, no shared state
- [ ] **Includes**: All necessary headers included; relative paths correct
- [ ] **Compilation**: Code compiles with `run.bat build` (zero errors, zero warnings)
- [ ] **Execution**: All tests pass with `run.bat test`
- [ ] **Coverage**: Measurement shows C0=100%, C1=100%, C2=100%
- [ ] **Report**: HTML coverage report generated successfully

---

## 15. Test Case Consolidation and Limiting Test Quantity

### CRITICAL RULE: Consolidate Similar Tests Using TEST_P

**Problem**: Writing dozens of nearly-identical TEST_F cases with different numbers is inefficient and violates DRY principle.

**Solution**: Use parameterized tests (TEST_P) when you have:
- Same function being tested
- Same assertion pattern
- Only the input/output values differ

### Anti-Pattern: Too Many Individual Tests

❌ **BAD** — 25 separate TEST_F cases, each with different numeric inputs:
```cpp
TEST_F(CalculatorUnitTest, AddThree_AllPositive_ReturnsCorrectSum) {
    int result = calculator.addThree(5, 3, 2);
    EXPECT_EQ(result, 10);
}
TEST_F(CalculatorUnitTest, AddThree_AllPositive_LargeNumbers) {
    int result = calculator.addThree(100, 200, 300);
    EXPECT_EQ(result, 600);
}
TEST_F(CalculatorUnitTest, AddThree_AllPositive_SingleDigits) {
    int result = calculator.addThree(1, 1, 1);
    EXPECT_EQ(result, 3);
}
// ... 22 more identical cases ...
```

### Pattern: Consolidated Parameterized Tests

✅ **GOOD** — 1 parameterized test covering all value variations:
```cpp
class CalculatorAddThreeTest : public ::testing::TestWithParam<std::tuple<int, int, int, int>> {
protected:
    Calculator calculator;
};

INSTANTIATE_TEST_SUITE_P(
    Normal_Cases,
    CalculatorAddThreeTest,
    ::testing::Values(
        std::make_tuple(5, 3, 2, 10),           // Small positive
        std::make_tuple(100, 200, 300, 600),    // Large positive
        std::make_tuple(1, 1, 1, 3),            // Minimal positive
        std::make_tuple(-5, -3, -2, -10),       // Negative
        std::make_tuple(0, 0, 0, 0),            // Zero
        std::make_tuple(INT_MAX/2, INT_MAX/2, 0, INT_MAX-1), // Near boundary
        std::make_tuple(INT_MIN/2, INT_MIN/2, 0, INT_MIN+1)  // Near min boundary
    )
);

TEST_P(CalculatorAddThreeTest, SWE4_Calculator_addThree_Comprehensive_Values) {
    auto [a, b, c, expected] = GetParam();
    int actual = calculator.addThree(a, b, c);
    EXPECT_EQ(actual, expected);
}
```

### Guidelines for Test Quantity

| Test Category | Recommended Count | Rule |
|---------------|-------------------|------|
| **Normal Cases** | Only when required by SDD/requirements (0-2) | Use TEST_P only if the requirement/SDD mandates specific input values; otherwise do not add extra numeric variants |
| **Boundary Cases** | ~5-6 | Use TEST_P for the STRICT set: 0, INT_MAX, INT_MIN, INT_MAX/2, INT_MIN/2 (optional near-boundary to reach 6) |
| **Error Cases** | 2-4 | Use TEST_F for complex error scenarios; TEST_P for value variations |
| **Total per Function** | **10-15** | Consolidate with TEST_P to avoid test explosion |
| **Total per Module** | **20-40** | If > 40 tests, consolidate further or review if all are necessary |

**GOAL**: Cover 100% C0/C1/C2 with minimal redundant test cases.

### When to REFUSE Test Case Consolidation

✅ **Keep separate TEST_F cases when**:
- Logic differs significantly (e.g., different assertion types: `EXPECT_EQ` vs `EXPECT_TRUE`)
- Test has unique setup/teardown requirements
- Mock configuration differs between tests
- Testing distinct code paths (branches/conditions)
- Error handling requires special preconditions

Example: Different Assertions = Separate Tests
```cpp
TEST_F(CalculatorUnitTest, SWE4_Calculator_validate_Normal_Valid_Input) {
    // Uses EXPECT_TRUE for boolean validation
    bool result = calculator.validate(5, 10);
    EXPECT_TRUE(result);
}

TEST_F(CalculatorUnitTest, SWE4_Calculator_validate_Error_Invalid_Input) {
    // Uses EXPECT_FALSE for invalid case (different assertion)
    bool result = calculator.validate(-5, 10);
    EXPECT_FALSE(result);
}
// Cannot consolidate: EXPECT_TRUE vs EXPECT_FALSE are fundamentally different
```

---

## 16. Iterations and Coverage Improvement (Max 5 Attempts)

### Iteration Workflow
1. **Measure Coverage**: Run `run.bat report` to generate standard LCOV HTML reports in `reports/`
2. **Analyze Gaps**: Open `reports/coverage_<module>.html` and identify lines/branches with 0 hits (shown in red)
3. **Add Test Cases**: Create new tests targeting uncovered code paths (reference line numbers from HTML report)
4. **Rebuild & Retest**: Run `run.bat build` and `run.bat test`
5. **Repeat**: Continue iterations until 100% or 5 attempts exhausted

### Coverage Gap Analysis Template
```
## Iteration N Coverage Report

### Summary
- C0 Coverage: X% (Y lines uncovered, see coverage_<module>.html for details)
- C1 Coverage: Z% (N branches uncovered)
- C2 Coverage: W% (M condition combinations uncovered)

### Uncovered Lines Analysis
| Line | Code | Reason | Planned Fix |
|------|------|--------|------------|
| 42 | `if (a < 0)` | Not tested (red in HTML report) | Add test with negative input |

### Added Test Cases
- Added: SWE4_Calculator_complexAdd_Error_Negative_Boundary_Negative_Max
- Added: SWE4_Calculator_complexAdd_Boundary_Near_Zero_Negative_One

### Next Steps
Recompile and regenerate LCOV report to verify coverage improvement
```

---

## 16. Test Case Minimization Strategy (Achieving Coverage with Minimal Redundancy)

### Core Principle: "Minimum Sufficient Tests"

Generate **only the test cases necessary to achieve 100% C0/C1/C2 coverage**. Eliminate redundant tests that don't add new coverage paths. This section provides rules for each function pattern.

### 16.1 Classification by Function Complexity

Determine your function type and apply the corresponding minimum test strategy:

#### **Type A: Linear Functions (No Branches, No Conditions)**
**Example**: `int addThree(int a, int b, int c) { return a + b + c; }`

**Coverage Requirements**:
- C0: Execute the single statement ✅ 1 test sufficient
- C1: No branches ✅ N/A
- C2: No conditions ✅ N/A

**MINIMUM Test Cases**:
| Test Type | Count | Rationale |
|-----------|-------|-----------|
| Normal case | 0-1 (only if SDD/requirement mandates) | Include only when specific input values are required by SDD; otherwise omit |
| Boundary (zero) | 1 | Verify 0 behavior (required per boundary policy) |
| Boundary (large) | 1 | Verify scale independence (use the strict boundary set in §7) |
| **TOTAL** | **1-3** | Minimal tests; prefer boundary-only unless SDD requires normal values |

**Anti-Pattern** ❌:
```cpp
// WRONG: Too many redundant tests for linear function
TEST_P(..., 7 variants of small/large/positive/negative)  // Overkill
TEST_F(..., Commutative property test)                     // Math property, not code coverage
TEST_F(..., Associative property test)                     // Math property, not code coverage
TEST_F(..., Zero identity test)                            // Covered by boundary test
// Total: 26 tests for 2 lines of code!
```

**Correct Pattern** ✅:
```cpp
// CORRECT: Minimal tests for linear function
INSTANTIATE_TEST_SUITE_P(
    SWE4_Calculator_Minimal_Coverage,
    CalculatorTest,
    ::testing::Values(
        std::make_tuple(5, 3, 2, 10),              // Normal case (C0)
        std::make_tuple(0, 0, 0, 0),               // Boundary: all zero
        std::make_tuple(INT_MAX/4, INT_MAX/4, INT_MAX/4, (INT_MAX/4)*3)  // Boundary: large
    )
);
TEST_P(..., SWE4_Calculator_addThree_Minimal_Coverage) { ... }
// Total: 3 tests, 100% C0 coverage
```

---

#### **Type B: Functions with Branches (if/else, switch)**
**Example**: 
```cpp
int classify(int x) {
    if (x < 0) return -1;
    if (x == 0) return 0;
    return 1;
}
```

**Coverage Requirements**:
- C0: Execute all statements ✅ Each branch path
- C1: Both paths of each if ✅ True AND false for each condition
- C2: All condition combinations ✅ All relevant value combinations

**MINIMUM Test Cases per Branch**:
| Branch | Test Case | Expected | Purpose |
|--------|-----------|----------|---------|
| `x < 0` TRUE | x = -1 | -1 | True path of condition |
| `x < 0` FALSE, `x == 0` TRUE | x = 0 | 0 | False of first, true of second |
| `x < 0` FALSE, `x == 0` FALSE | x = 1 | 1 | Default path |
| **TOTAL** | **3** | | C0/C1=100% |

**Implementation**:
```cpp
INSTANTIATE_TEST_SUITE_P(
    SWE4_Classify_Branch_Coverage,
    ClassifyTest,
    ::testing::Values(
        std::make_tuple(-1, -1),   // Branch: x < 0
        std::make_tuple(0, 0),     // Branch: x == 0
        std::make_tuple(1, 1)      // Branch: default
    )
);
```

**DO NOT add extra tests** for mathematically different values (e.g., -5, -100, -INT_MAX) unless each hits a **different branch**. If they all hit `x < 0` branch, they're redundant.

---

#### **Type C: Functions with Loops**
**Example**:
```cpp
int sumArray(int* arr, int count) {
    int sum = 0;
    for (int i = 0; i < count; i++) sum += arr[i];
    return sum;
}
```

**Coverage Requirements**:
- C0: Loop body executed (at least once) ✅ count ≥ 1
- C1: Loop iterations (0, 1, many) ✅ count=0, count=1, count≥2
- C2: Loop exit condition (true, false) ✅ Iterations tested above

**MINIMUM Test Cases**:
| Test | Count | Array | Expected | Purpose |
|------|-------|-------|----------|---------|
| No iterations | 0 | {} | 0 | Loop: count=0 (skip body) |
| One iteration | 1 | {5} | 5 | Loop: count=1 (execute once) |
| Many iterations | 3 | {1,2,3} | 6 | Loop: count≥2 (multiple) |
| **TOTAL** | **3** | | | C0/C1/C2=100% |

**Consolidate with TEST_P**:
```cpp
INSTANTIATE_TEST_SUITE_P(
    SWE4_SumArray_Loop_Coverage,
    SumArrayTest,
    ::testing::Values(
        std::make_tuple(std::vector<int>{}, 0),           // 0 iterations
        std::make_tuple(std::vector<int>{5}, 5),          // 1 iteration
        std::make_tuple(std::vector<int>{1,2,3}, 6)       // 3 iterations
    )
);
```

---

#### **Type D: Complex Functions (Branches + Loops + Conditions)**
**Example**:
```cpp
int processData(int* data, int size) {
    if (size <= 0) return -1;
    int result = 0;
    for (int i = 0; i < size; i++) {
        if (data[i] > 100) result += data[i];
    }
    return result;
}
```

**Coverage Strategy**:
1. **Branch paths**: `size <= 0` (true, false)
2. **Loop iterations**: 0, 1, many
3. **Inner condition**: `data[i] > 100` (true, false per iteration)

**MINIMUM Test Cases** (Cartesian product):
- size <= 0 → return early (1 test, all branches covered)
- size = 1, data[0] > 100 → true (1 test, inner if TRUE)
- size = 1, data[0] <= 100 → false (1 test, inner if FALSE)  
- size = 3, mix > and <= 100 values → tests multiple iterations

**Total: 4 tests** for complete C0/C1/C2 coverage.

**Implementation**:
```cpp
INSTANTIATE_TEST_SUITE_P(
    SWE4_ProcessData_Complex_Coverage,
    ProcessDataTest,
    ::testing::Values(
        std::make_tuple(std::vector<int>{}, 0, -1),           // size=0: early return
        std::make_tuple(std::vector<int>{150}, 1, 150),       // size=1, >100: inner if TRUE
        std::make_tuple(std::vector<int>{50}, 1, 0),          // size=1, <=100: inner if FALSE
        std::make_tuple(std::vector<int>{150,50,200}, 3, 350) // size=3: multiple iterations + mixed condition
    )
);
```

---

### 16.2 Coverage-Driven Test Design (NOT Property-Based Testing)

**KEY DISTINCTION**:
- ✅ **Coverage-Driven**: "Test what branches/conditions exist" → Minimize tests
- ❌ **Property-Based**: "Test mathematical properties" → Over-tests

### What NOT to Do: Property Tests

These add NO coverage value (code doesn't branch on them):

```cpp
// ❌ WRONG: Mathematical property test (no new coverage)
TEST_F(Calculator, SWE4_Add_Commutative_Order_Independence) {
    EXPECT_EQ(add(a, b), add(b, a));  // This property is true because of math, not code branches
}

// ❌ WRONG: Identity property (no new coverage)
TEST_F(Calculator, SWE4_Add_Zero_Identity) {
    EXPECT_EQ(add(a, 0), a);  // Zero behavior is math, not code logic
}

// ❌ WRONG: Associative property (no new coverage)
TEST_F(Calculator, SWE4_Add_Associative) {
    EXPECT_EQ(add(add(a,b),c), add(a,add(b,c)));  // True by math, not code
}
```

Each of these tests exercises the **same single code path**: the addition operation. They don't hit different branches or conditions—they just use different input values that happen to demonstrate mathematical properties. **They add zero code coverage value beyond the first normal case test.**

### What TO Do: Coverage-Driven Tests

```cpp
// ✅ CORRECT: Minimal coverage-driven tests
INSTANTIATE_TEST_SUITE_P(
    SWE4_Add_Coverage,
    AddTest,
    ::testing::Values(
        std::make_tuple(5, 3, 8),                // Normal case: C0 coverage
        std::make_tuple(0, 0, 0),                // Boundary: zero behavior
        std::make_tuple(INT_MAX/2, INT_MAX/2, INT_MAX)  // Boundary: large values
    )
);
TEST_P(AddTest, SWE4_Add_Coverage) { ... }

// Total: 3 tests, 100% C0 coverage
// No redundant property tests
```

---

### 16.3 Boundary Value Selection Rules

**When to include boundary tests**:

| Boundary Type | Include? | Example | Rationale |
|---------------|----------|---------|-----------|
| **Zero** | ✅ YES | 0 | Additive identity; asymptotic behavior |
| **INT_MAX** | ✅ YES (STRICT)** | INT_MAX, INT_MAX/2 | Required for numeric boundary analysis |
| **INT_MIN** | ✅ YES (STRICT)** | INT_MIN, INT_MIN/2 | Required for numeric boundary analysis |
| **Loop: 0 iterations** | ✅ YES | count=0 | May skip loop body entirely |
| **Loop: 1 iteration** | ✅ YES | count=1 | Off-by-one errors |
| **Loop: Many iterations** | ✅ YES | count=100 | Loop accumulation patterns |
| **Negative numbers** | ✅ YES (if used) | -5, -100 | Sign-dependent behavior |
| **Small positive** | ⚠️ OPTIONAL | 1, 5 | Only if distinct from large positive |
| **Large positive** | ⚠️ OPTIONAL | 1000, 10000 | Only if distinct from small positive |

**Example**: For `addThree(a, b, c)`:
- ✅ Include (STRICT minimal): 0, INT_MAX, INT_MIN, INT_MAX/2, INT_MIN/2
- ✅ Optional (if needed): one near-boundary case such as `INT_MAX-1` or `INT_MIN+1` (to reach 6 total)
- ❌ Exclude: many extra numeric variants that do not exercise new branches (e.g., (0,1), (1,2), (2,3))

---

### 16.4 Test Count Limits (Hard Caps)

To prevent test suite bloat:

| Limit | Rule | Reason |
|-------|------|--------|
| **Per Function** | ≤ 15 test cases | Prevents redundancy; encourages consolidation |
| **Per Module** | ≤ 40 test cases | Keeps test suite maintainable |
| **Parameterized Suite** | ≤ 10 parameter sets | Too many variants → split into separate suites |

**How to consolidate when approaching limits**:

```cpp
// ❌ WRONG: 12 separate TEST_F functions for same logic
TEST_F(CalcTest, SWE4_Add_Normal_Small_Pos) { ... }
TEST_F(CalcTest, SWE4_Add_Normal_Large_Pos) { ... }
TEST_F(CalcTest, SWE4_Add_Normal_Small_Neg) { ... }
// ... 9 more variations

// ✅ CORRECT: Consolidate variants into single TEST_P
INSTANTIATE_TEST_SUITE_P(
    SWE4_Add_Variants,
    AddParameterizedTest,
    ::testing::Values(
        std::make_tuple(5, 3, 8),       // small positive
        std::make_tuple(1000, 2000, 3000),  // large positive
        std::make_tuple(-5, -3, -8),    // small negative
        std::make_tuple(-1000, -2000, -3000) // large negative
        // ... etc, consolidated into ONE TEST_P with 4 variants
    )
);
TEST_P(AddParameterizedTest, SWE4_Add_Variants) { ... }
// Total: 1 test method, 4 parameter sets
```

---

### 16.5 Decision Tree: How Many Tests Do I Need?

```
START: Do I have branches/conditions in my function?
│
├─ NO (linear function, like addThree)
│  └─ TESTS NEEDED:
│     ├─ 1× Normal case (to hit C0)
│     ├─ 1× Boundary zero (if applicable)
│     └─ 1× Boundary large (if applicable)
│     TOTAL: 2-3 tests
│
└─ YES (if statements or switch)
   │
   └─ How many distinct code paths?
      │
      ├─ 2 paths (single if/else)
      │  └─ TESTS: 2 tests (one for each branch)
      │
      ├─ 3-4 paths (multiple ifs or switch)
      │  └─ TESTS: 3-4 tests (one per path)
      │
      └─ Complex (combinations of branches, loops)
         └─ TESTS: (# of paths) × (optional loop iteration variants)
            Example: 2 branch paths × 3 loop variants = 6 tests MAX

END: Consolidate all parameter variants using TEST_P
     Limit total count: ≤15 per function, ≤40 per module
```

---

### 16.6 Documentation: When Adding Extra Tests

If you add more than the calculated minimum, **document why**:

```cpp
// GOOD: Explains why extra test was added
/// ===========================================================================
/// Verifies: Calculator::complexDivide()
/// Test goal: Validates division by small positive (different from zero division)
/// In case: Create Calculator, call complexDivide(100, 2), verify result = 50
/// Method: Distinct from zero-division boundary; small divisor edge case
/// ===========================================================================
TEST_F(CalcTest, SWE4_Calc_complexDivide_Boundary_Small_Divisor) { ... }

// BAD: No justification for extra test
TEST_F(CalcTest, SWE4_Calc_complexDivide_Normal_Value_15) { ... }
TEST_F(CalcTest, SWE4_Calc_complexDivide_Normal_Value_16) { ... }
```

---

## 17. Final Acceptance Criteria

A test file is complete and acceptable when:
1. ✅ Compiles without errors or warnings
2. ✅ All test cases pass (100% test pass rate)
3. ✅ C0/C1/C2 coverage = 100%, OR if < 100% after 5 iterations, all uncovered lines documented with technical justification (§11)
4. ✅ HTML coverage report generated successfully
5. ✅ All test names follow SWE4 naming convention (§1)
6. ✅ All tests have documentation headers per §2 template
7. ✅ No magic numbers; all test data named with constants (§9)
8. ✅ Tests are independent; can run in any order (§10.4)
9. ✅ Mocking strategy is appropriate; external dependencies mocked, internal use real instances (§6)
10. ✅ ONLY TEST_F and TEST_P macros used — NO TEST() macros (§7)
11. ✅ All boundary values justified per §16.3 (zero, INT_MAX/MIN, loop iteration counts)
12. ✅ Private functions tested exclusively through public interface (§10.1)
13. ✅ Private variables tested via getters/setters only; NO friend declarations (§10.2-10.3)
14. ✅ **Similar test cases consolidated using TEST_P** — Same function, same logic, different values → Single TEST_P (§16.2)
15. ✅ **Test quantity within limits**: ≤15 per function, ≤40 per module; NO property-based tests (§16.4)
16. ✅ **NO redundant property tests** — Commutative, associative, identity properties not tested unless they hit different code branches (§16.2)
17. ✅ **Only coverage-driven tests** — Each test must exercise a distinct code path, branch, or condition (§16.2)
18. ✅ **Overflow/underflow tested** (if numeric): Tests approach INT_MAX/INT_MIN with documented behavior (§7.2)
19. ✅ **ZERO modifications to src/ and inc/ directories** ⚠️ MOST CRITICAL — Production code completely untouched (CRITICAL RULE #1)
20. ✅ Test file size reasonable — Typically 150-300 lines for simple modules, <500 for complex ones
