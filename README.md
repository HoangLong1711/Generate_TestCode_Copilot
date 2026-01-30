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

## 🧠 How It Works

The core idea is to **guide Copilot with a predefined agent instruction** so that it understands:

- What kind of test to generate
- Which module to focus on
- The expected output format and structure

Copilot then acts as an **AI test generator**, not just a code completer.

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
