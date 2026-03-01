# Reviewing Rules Checklist

Purpose: provide a concise, editable checklist used by the automated review step to verify that generated tests follow the rules defined in `Testing_Rules.md`. The agent will run a 5-pass automated review against this checklist and prompt the user to correct any mismatches.

**How to use**
- Update checklist items to match your project's SDD/requirements and any local conventions.
- Each automated review pass will mark items as: `PASS`, `FAIL`, or `N/A` (not applicable).
- If any required item is `FAIL` after the automated review attempts, the agent will prompt the user to update tests or the rules.

**Review Iterations**
- The automated agent runs up to 5 review iterations. After each iteration it will attempt minor auto-fixes where safe (e.g., consolidating TEST_P usage) and re-run the checks. If failures remain after 5 iterations, the agent stops and requests user intervention.

**Checklist** (edit as needed)

- **Traceability:** All numeric test values are traceable to a requirement or SDD clause (trace ID present in the test header).
- **Boundary Set:** Boundary tests include only the strict set: `0`, `INT_MAX`, `INT_MIN`, `INT_MAX/2`, `INT_MIN/2`, (optional near-boundary).
- **Normal Cases:** Normal/value-specific tests exist only when required by SDD/requirements (0–2 cases); otherwise none.
- **No Production Edits:** No modifications were made to files under `src/` or `inc/`.
- **TEST_P Consolidation:** Parameterized tests (`TEST_P`) are used instead of many redundant `TEST_F` cases where only numeric parameters vary.
- **Documentation Headers:** Every test has the required documentation header (Verifies/Goal/In case/Method).
- **Mocking Strategy:** External dependencies are mocked; internal methods use real instances unless SDD specifies otherwise.
- **Coverage Targets:** Coverage report exists and shows C0/C1/C2 metrics; if <100% the specific reasons for uncovered lines (e.g., code flow limitations, access conditions) are automatically generated and appended to the central `reports/report/coverage_limitations.txt` per rules.
- **No JS Fallback:** Agent did not generate JS/HTML/CSS analysis fallback in place of a native build; instead it listed missing toolchain dependencies if build tools were absent.
- **Test Naming:** Test file and test names follow the `SWE4_<ModuleName>` and `SWE4_<Module>_<Function>_<Scenario>` conventions.
- **Quantity Limits:** Per-function and per-module test counts respect the limits in `Testing_Rules.md` (≤15 per function, ≤40 per module) unless justified and documented.
- **SDD/REQ Placeholders:** Placeholder requirement IDs are present where needed and highlighted for user replacement.

**Review Notes / Actions**
- For any `FAIL`, the agent will attempt one of:
  - Consolidate similar tests using `TEST_P` (if safe)
  - Remove redundant numeric variants that lack traceability
  - Add missing documentation header templates (does not invent SDD IDs)
- If agent cannot safely auto-fix a `FAIL`, it will prompt the user with a concise remediation list.

---
Editable by project leads; keep this file under version control and update as SDD/rules evolve.
