# Plan: Google Test Workflow for C++ Unit Testing

This workflow provides a step-by-step process to generate, build, execute, and validate Google Test test cases with 100% code coverage following ISO 26262 ASIL D standards. The workflow integrates with your existing CMake build system, gcovr coverage analysis, and naming conventions (SWE4_<ModuleName>.cpp).

## Steps

1. **Parse input file and validate test target** — Accept src/*.cpp or inc/*.hpp path, extract module name, validate file exists, and determine test output filename (SWE4_<ModuleName>.cpp)

2. **Recursively scan and document dependencies** — Read the target file, extract all #include directives, recursively trace included files to build complete dependency graph (headers, implementations, mocks needed)

3. **Generate test cases using SWE4 rules** — Read [Testing_Rules.md](Testing_Rules.md) to retrieve comprehensive testing standards and generate test cases following these rules:
   - Class name: `<ModuleName>UnitTest : public ::testing::Test`
   - Per-function naming: `SWE4_<ModuleName>_<FunctionName>_<Scenario>`
   - Doc headers with Verifies/Goal/Setup/Method sections (as defined in Testing_Rules.md §2)
   - Normal, Boundary, and Error cases for each function (as defined in Testing_Rules.md §5)
   - Mock external dependencies using `NiceMock<Stub>` + `EXPECT_CALL` (as defined in Testing_Rules.md §6)
   - Ensure 100% C0/C1/C2 coverage target per Testing_Rules.md §4

4. **Write SWE4_<ModuleName>.cpp and compile (Auto-fix until success)** — Save generated tests to test/ folder, run `run.bat build` to compile; if compilation fails, automatically detect compiler/linker errors, fix issues in test file, and rebuild; **repeat until build succeeds with NO iteration limit** (only coverage iterations are limited to 5)
   - **Note**: Never modify `run.bat` or CMakeLists.txt; only fix test code itself
   - If `run.bat build` fails due to missing dependencies, report which libraries/tools are missing and halt; user must install them manually

5. **Execute tests and analyze coverage** — Clean old coverage data first (remove build/*.gcda, build/*.gcov, reports/*). Run `run.bat test` to execute the test suite. Then run `run.bat report [<module>]` to generate a gcovr coverage report (C0/C1/C2); if you supply a module name it will be placed in `reports/<module>/coverage.html`, otherwise `reports/general/coverage.html`. Parse results to identify uncovered lines/branches.
   - **If `run.bat test` fails**: Report error; user must fix environment (likely missing gtest/gmock dependency)
   - **If `run.bat report` fails**: Report missing gcovr tool; user must install before coverage analysis can proceed

6. **Iterate test improvements (max 5 attempts for coverage only)** — If coverage < 100%, refine test cases for uncovered paths, rebuild (using unlimited auto-fix for any new build errors in test code), re-test, and re-measure coverage; **after 5 coverage improvement iterations**, document all uncovered lines with explanations (unreachable code, conditional complexity, etc.) in the HTML report
   - **Build failures during iteration**: Fix test code issues; if failure is due to missing dependencies, report to user and halt iteration

7. **Automated Rules Mapping Review (5 passes)** — After coverage iterations complete, run an automated review that compares generated tests and metadata against the rules in [Testing_Rules.md](Testing_Rules.md) and the checklist in [reviewing_Rules.md](reviewing_Rules.md). The agent will attempt up to 5 automated review passes. On each pass it will report mismatches and attempt safe auto-fixes (e.g., consolidate `TEST_P`, add missing documentation headers, remove un-traced numeric cases). If issues remain after 5 passes, the agent will pause and present a concise remediation list and prompt the user to correct tests or rules before proceeding.

## Further Considerations

1. **Dependency Mocking** — Should external dependencies (e.g., `mt::Math`, Device parent classes) be mocked for isolation, or use real implementations? (Recommendation: Mock external; test internal calls via real implementations)

2. **Test Isolation** — Do you want parameterized tests (`TEST_P`) for boundary value analysis, or separate `TEST_F` cases? (Recommendation: Mixed approach — `TEST_P` for numeric ranges, `TEST_F` for complex scenarios)

3. **Coverage Reporting** — Currently gcovr filters to src/ + inc/; should test/ files themselves be included in coverage metrics? (Recommendation: Exclude test/ from coverage; focus on production code C0/C1/C2)
