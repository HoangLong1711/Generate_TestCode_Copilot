# Generate_TestCode_Copilot

An AI-assisted tool built on top of **GitHub Copilot Agents in VS Code** to automatically generate **unit test scripts** for a **specific module** with minimal manual effort.

This tool is designed to help developers quickly bootstrap unit tests by leveraging Copilot as an **AI agent**, guided by a predefined instruction file.

---

## 🚀 Overview

`Generate_TestCode_Copilot` helps you generate unit test code automatically by:

- Using **GitHub Copilot AI Agent** in VS Code
- Providing a reusable **agent instruction file**
- Generating test scripts **for a specific module only**
- Reducing repetitive and boilerplate test writing

The workflow is intentionally simple and developer-friendly.

---

## ✨ Key Features

- 🤖 AI-powered test generation via **Copilot Agent**
- 🎯 Targeted test generation for **specific modules**
- ⚡ Minimal setup, no complex configuration
- 🧩 Easy to extend for different projects or test frameworks
- 🛠️ Written in **C++-friendly structure** (can be adapted)

---

## 📦 Prerequisites (What to Install)

To run this tool on a new computer, you need to install the following software and ensure they are added to your system's `PATH`:

### Required
- **[CMake](https://cmake.org/download/)** (v3.14+): Used to configure the build system.
- **C++ Compiler & Make tool**: We highly recommend **[MinGW-w64 (GCC)](https://www.msys2.org/)** as the `run.bat` script natively relies on `mingw32-make` and `MinGW Makefiles`.

### Optional (For Code Coverage Reports)
- **[LLVM/Clang](https://releases.llvm.org/)** (`clang++`, `llvm-profdata`, `llvm-cov`): Recommended for faster, source-based HTML coverage reporting.
- **[Python](https://www.python.org/downloads/) & [gcovr](https://gcovr.com/)** (run `pip install gcovr`): Used as a fallback coverage reporting tool if LLVM is not available.

*(Note: The **Google Test/Mock** framework is downloaded automatically by CMake during the build process, so no manual installation is required.)*

---

## 🧠 How It Works

The core idea is to **guide Copilot with a predefined agent instruction** so that it understands:

- What kind of test to generate
- Which module to focus on
- The expected output format and structure

Copilot then acts as an **AI test generator**, not just a code completer.

---
## 🧪 Step-by-Step: How to Use Copilot Agent

This section explains how to use **Generate_TestCode_Copilot** to automatically generate unit test scripts using **GitHub Copilot Agent in VS Code**.

## How to Use

1. Open the source file you want to generate unit tests for in VS Code.

2. In Copilot Chat (Agent mode), use the following command:

    **--> Generate for this file + <module_name>**

3. Check the generated test script in the **test** folder.

4. Check the coverage report in the **report** folder.

---

## run.bat Commands

The `run.bat` script is a helper utility that automates the **build**, **test**, and **report generation** workflow. It supports flexible command-line arguments to let teams easily generate tests and coverage reports for different modules.

### Available Commands

#### 1. **`run.bat build [module]`**
Builds the project and prepares it for testing.

- **Usage**: `run.bat build`
- **What it does**:
  - Clears and recreates the `build/` directory
  - Detects and configures CMake with available compilers (Clang, GCC/MinGW)
  - Enables LLVM coverage tools if available, falls back to GCC/gcov otherwise
  - Compiles all source code and test files
- **Example**: `run.bat build` — Standard build for all modules

#### 2. **`run.bat test [module]`**
Runs all unit tests and generates profiling data.

- **Usage**: `run.bat test`
- **What it does**:
  - Executes the compiled test executable (`run_tests.exe`)
  - Collects code coverage data (LLVM profiling or GCC coverage)
  - Exits with error code if any tests fail
- **Example**: `run.bat test` — Run all tests

#### 3. **`run.bat report [module]`**
Generates HTML coverage reports from test execution data.

- **Usage**: `run.bat report [module_name]`
- **What it does**:
  - Processes coverage data (from LLVM or gcov)
  - Generates an interactive HTML coverage report
  - Stores reports in `reports/[module]/`
  - Reports show which lines of code were tested and coverage percentages
- **Examples**:
  - `run.bat report` — Generate report for "general" module
  - `run.bat report AccountManager` — Generate report for AccountManager module

#### 4. **`run.bat all [module]`** (Default)
Runs the complete workflow: **build → test → report**.

- **Usage**: `run.bat all` or simply `run.bat` (default)
- **What it does**:
  - Executes build, test, and report steps in sequence
  - Stops immediately if any step fails
  - Generates complete test coverage report in `reports/` folder
- **Examples**:
  - `run.bat all` — Complete workflow for "general" module
  - `run.bat all TransactionProcessor` — Complete workflow for TransactionProcessor module
  - `run.bat` — Same as `run.bat all` (default behavior)

### Typical Workflow Examples

#### Example 1: Generate tests for a new module
```batch
rem After creating new source and header files for YourModule

rem Option 1: Manual steps
run.bat build
run.bat test
run.bat report YourModule

rem Option 2: Automated (recommended)
run.bat all YourModule
```

#### Example 2: For CI/CD or automated testing
```batch
rem Run full workflow with a specific module name
run.bat all AccountManager

rem View the report in: reports/AccountManager/index.html or coverage.html
```

#### Example 3: Quick rebuild and test after code changes
```batch
run.bat all
```

### How New Users Can Use This Tool

1. **Write or generate source code** for your module in `src/` and create headers in `inc/`

2. **Generate unit tests** using Copilot:
   - Open your source file (e.g., `src/MyModule.cpp`)
   - In Copilot Chat (Agent mode), type: `Generate for this file + MyModule`
   - Tests will be created in `test/SWE4_MyModule.cpp`

3. **Build and test automatically**:
   ```batch
   run.bat all MyModule
   ```

4. **Review the coverage report**:
   - Open `reports/MyModule/index.html` in your browser
   - See which functions and lines were tested

### Notes

- **Module name**: The `[module]` parameter is optional. If not provided, defaults to `"general"`
- **Compiler detection**: The script automatically detects Clang/LLVM or GCC and configures accordingly
- **Fallback coverage**: If LLVM tools aren't available, the script falls back to GCC/gcov/gcovr
- **Exit codes**: Non-zero exit codes indicate failures (useful for CI/CD pipelines)

---

## 📂 Project Structure

```text
Generate_TestCode_Copilot/
│
├─ .github/                 # GitHub configuration (PR templates, workflows, etc.)
│
├─ src/                     # Source code (if applicable)
│
├─ inc/                     # Header files
│
├─ test/                    # Generated / manual unit tests
│
├─ reports/                 # Test reports / coverage outputs
│
├─ build/                   # Build artifacts (can be ignored in .gitignore)
│
├─ UT_Support.agent         # ⭐ Copilot AI Agent instruction file
│
├─ CMakeLists.txt           # CMake build configuration
│
├─ run.bat                  # Helper script to run the tool or tests
│
└─ README.md                # Project documentation
