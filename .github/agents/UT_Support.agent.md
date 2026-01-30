---
description: 'Google Test Workflow for C++ Unit Testing - Generates comprehensive ISO 26262 ASIL D compliant unit tests.'
tools: ['execute', 'read', 'edit', 'search', 'web', 'agent', 'ms-python.python/getPythonEnvironmentInfo', 'ms-python.python/getPythonExecutableCommand', 'ms-python.python/installPythonPackage', 'ms-python.python/configurePythonEnvironment']
---

## What This Agent Accomplishes

The **UT_Support Agent** automates comprehensive Google Test unit test generation and validation for C++ source files. Simply provide file path(s) and the agent will:

1. **Parse input files** — Automatically extract module name from filename
2. **Execute full workflow** — Run the 6-step process from [plan-googleTestWorkflow.prompt.md](../prompts/plan-googleTestWorkflow.prompt.md)
3. **Generate optimized tests** — Apply [Testing_Rules.md](../prompts/Testing_Rules.md) standards with minimized test count
4. **Compile and validate** — Build with auto-error correction (unlimited attempts)
5. **Measure coverage** — Achieve C0/C1/C2 targets with minimal redundancy
6. **Generate reports** — HTML coverage reports + console metrics

---

## How to Use This Agent

**Command Pattern**: `Generate for this file <ModuleName>` or `Generate for this file <ModuleName1>, <ModuleName2>, ...`

### Single Module
**Input**: `Generate for this file Calculator`
- Agent automatically locates Calculator in src/ or inc/
- Executes full workflow for Calculator only
- Returns compiled test suite + coverage report
- Cleans up old coverage data before fresh run

### Multiple Modules (Batch Mode)
**Input**: `Generate for this file Calculator, Device`
- Agent processes Calculator first (full workflow → report)
- Then processes Device (full workflow → report)
- Cleans old data before each module to ensure fresh reports
- Returns separate results for each module

### Alternative: With File Path
**Input**: `Generate for this file src\Calculator.cpp` or `Generate for this file inc\Math.hpp`
- Agent accepts full paths or module names (auto-resolves)
- Extracts module name from filename
- Proceeds with test generation
- Cleans old coverage data for that module before run

---

## Edges This Agent Won't Cross

The agent **will not**:
- Modify production source code (src/ or inc/) — Critical Rule #1 in Testing_Rules.md
- Exceed 5 coverage improvement iterations — Documents uncovered code after 5 attempts (build error fixing has unlimited retries)
- Generate tests for external third-party libraries (GoogleTest/GoogleMock frameworks themselves)
- Perform integration testing beyond unit-level component isolation
- Guarantee code correctness — Only verifies coverage and structure; test logic validation is user responsibility
- Handle non-C++ files or unsupported architectures (assumes GCC/MinGW on Windows)

---

## Ideal Inputs/Outputs

### Input Format
```
File Path: src/Calculator.cpp  OR  inc/Calculator.hpp
Module Name: Calculator (auto-extracted from filename)
```

### Output Artifacts
1. **Test File**: `test/SWE4_<Module>.cpp` (optimized with TEST_P consolidation, §16 minimization rules)
2. **Build Result**: Successful compilation with `run_tests.exe` executable (auto-fixed all build errors)
3. **Test Results**: Pass/fail summary for all test cases
4. **Coverage Report** (Standard LCOV format): 
   - File: `reports/coverage_<Module>.html` (standard LCOV HTML)
   - Contents: Line coverage with hit counts, branch visualization, function metrics
   - Color-coding: Green (covered), Red (uncovered), Yellow (partial)
5. **Documentation**: 
   - If 100% coverage: Success summary with metrics
   - If <100% after 5 iterations: Uncovered code analysis with technical justifications per Testing_Rules.md §11

---

## How It Works

This agent executes the complete workflow defined in [plan-googleTestWorkflow.prompt.md](../prompts/plan-googleTestWorkflow.prompt.md):

1. **Parse and validate** input C++ file
2. **Scan dependencies** and build dependency graph  
3. **Generate test cases** following [Testing_Rules.md](../prompts/Testing_Rules.md) standards
4. **Compile** with automatic error fixing (unlimited retries)
5. **Execute tests** and measure coverage (C0/C1/C2)
6. **Iterate improvements** (max 5 attempts) or document uncovered code

The workflow applies all standards from Testing_Rules.md including:
- SWE4 test naming conventions
- Documentation headers for all tests
- Consolidation of similar tests using TEST_P with minimization rules (§16)
- Overflow/underflow boundary testing
- Test quantity limits (≤15 per function, ≤40 per module)
- Mocking strategy for external dependencies

### Data Freshness Guarantee
**Automatic cleanup before each run**:
- ✅ Clears old `.gcda` (coverage) files for the module
- ✅ Removes stale HTML reports in `reports/` for that module
- ✅ Deletes old `.o` object files from `build/` for clean recompilation
- ✅ Ensures new test run produces fresh, uncontaminated coverage data
- ✅ Prevents false positives from previous coverage runs

---

## Progress Reporting

The agent provides structured step-by-step feedback as it executes the workflow:

1. ✅ Old coverage data cleaned (fresh start)
2. ✅ File validated and module name extracted
3. ✅ Dependencies analyzed and documented
4. ✅ Test cases generated (consolidated using TEST_P per Testing_Rules.md §16 minimization rules)
5. ✅ Compilation successful (auto-fixed all build errors)
6. ✅ Coverage measured: C0=Y%, C1=Z%, C2=W% (Iteration N of max 5)
7. ✅ Report generated OR uncovered code documented

For detailed interaction points and help scenarios, see [plan-googleTestWorkflow.prompt.md](../prompts/plan-googleTestWorkflow.prompt.md#further-considerations)