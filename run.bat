@echo off
SET TASK=%1

IF "%TASK%"=="build" (
    if not exist build mkdir build
    pushd build
    cmake -G "MinGW Makefiles" ..
    cmake --build .
    popd
) ELSE IF "%TASK%"=="test" (
    if not exist build (
        echo Build directory not found. Run "build" first.
    ) else (
        pushd build
        .\run_tests.exe
        popd
    )
) ELSE IF "%TASK%"=="report" (
    if not exist reports mkdir reports
    REM Generate coverage reports with simple naming: coverage_<module>.html
    gcovr -r . --html -o reports/coverage_calculator.html --filter "src/" --gcov-ignore-errors=no_working_dir_found
    echo The report was created in the reports/ folder.
)