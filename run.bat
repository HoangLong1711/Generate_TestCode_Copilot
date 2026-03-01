@echo off
setlocal

rem Simple driver: build, test, report. Default TASK=all
set "TASK=%~1"
set "MODULE=%~2"
if "%TASK%"=="" set "TASK=all"
if "%MODULE%"=="" set "MODULE=general"

if /I "%TASK%"=="build" goto build
if /I "%TASK%"=="test" goto test
if /I "%TASK%"=="report" goto report
if /I "%TASK%"=="all" goto all

echo Usage: %~n0 [build|test|report|all] [module]
exit /b 1

:all
call :build
if errorlevel 1 exit /b %ERRORLEVEL%
call :test
if errorlevel 1 exit /b %ERRORLEVEL%
call :report %MODULE%
exit /b 0

:build
echo -- Build: creating build and running CMake

rem Clear RC environment variable to prevent CMake RC compiler errors
set "RC="

rem Kill any lingering test processes that might be locking files
taskkill /F /IM run_tests.exe /T >nul 2>&1
timeout /t 1 /nobreak >nul 2>&1

rem Remove old build directory if it exists to avoid file lock conflicts
if exist build (
    echo -- Removing old build directory
    rmdir /s /q build 2>nul
    timeout /t 1 /nobreak >nul 2>&1
)

if not exist build mkdir build
pushd build
rem detect clang and llvm tools to enable LLVM coverage build
set "USE_LLVM=0"
rem allow user to override clang location
if defined LLVM_CLANG_PATH (
    where "%LLVM_CLANG_PATH%" >nul 2>&1 && (
        set "PATH=%~dp0%LLVM_CLANG_PATH%;%PATH%"
    )
)
where clang++ >nul 2>&1
if %ERRORLEVEL%==0 (
    where llvm-profdata >nul 2>&1
    where llvm-cov >nul 2>&1
    if %ERRORLEVEL%==0 (
        set "USE_LLVM=1"
        rem verify clang can actually link a simple program
        set "EXTRA_FLAGS=%CLANG_EXTRA_FLAGS%"
        rem create simple test program using separate cmd call to avoid parser issues
        cmd /c "echo int main() { return 0; } > dummy.cpp"
        clang++ %EXTRA_FLAGS% "dummy.cpp" -o "dummy.exe" 2>"dummy.log"
        if %ERRORLEVEL% neq 0 (
            echo Warning: clang failed to link test program. See dummy.log for details.
            echo You must install a Windows SDK/VS or adjust CLANG_EXTRA_FLAGS so clang can find kernel32.lib etc.
            set "USE_LLVM=0"
        )
        del "dummy.cpp" 2>nul
        del "dummy.exe" 2>nul
        del "dummy.log" 2>nul
    )
)

if "%USE_LLVM%"=="1" (
    echo Detected clang/LLVM; attempting to configure CMake to use Clang with LLVM coverage
    rem if user supplied extra compiler flags (e.g. -target, -L paths), pass them through
    set "EXTRA_FLAGS=%CLANG_EXTRA_FLAGS%"
    if defined EXTRA_FLAGS (
        echo Using extra clang flags: %EXTRA_FLAGS%
        cmake -G "MinGW Makefiles" .. -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_CXX_FLAGS="%EXTRA_FLAGS%" -DCMAKE_C_FLAGS="%EXTRA_FLAGS%" -DUSE_LLVM_COVERAGE=ON
    ) else (
        cmake -G "MinGW Makefiles" .. -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DUSE_LLVM_COVERAGE=ON
    )
    if errorlevel 1 (
        echo Clang configuration failed; LLVM coverage unavailable in this environment.
        echo Please install a matching Windows SDK or use a clang toolchain targeting mingw.
        echo Falling back to default compiler ^(GCC^) with gcov/gcovr coverage.
        popd
        rd /s /q build 2>nul
        mkdir build
        pushd build
        cmake -G "MinGW Makefiles" .. -DUSE_LLVM_COVERAGE=OFF
    )
) else (
    cmake -G "MinGW Makefiles" .. -DUSE_LLVM_COVERAGE=OFF
)
if errorlevel 1 (
    echo CMake configuration failed.
    popd
    exit /b 1
)

rem Clean old build artifacts before building to avoid permission errors
echo -- Cleaning old build artifacts
mingw32-make clean >nul 2>&1

rem Build the project with parallel jobs
echo -- Compiling (this may take a minute)
mingw32-make -j4
set "RC=%ERRORLEVEL%"
popd
if %RC% neq 0 (
    echo Build failed with code %RC%.
    exit /b %RC%
)
exit /b 0

:test
echo -- Test: running unit tests
if not exist build (
    echo Build directory not found. Run "build" first.
    exit /b 1
)
pushd build
rem if LLVM tools present or build used clang, set profile output
where llvm-profdata >nul 2>&1
if %ERRORLEVEL%==0 (
    set "LLVM_PROFILE_FILE=default.profraw"
) else (
    rem no llvm-profdata found; tests will run without LLVM profiling
)
if exist run_tests.exe (
    .\run_tests.exe
    set "RC=%ERRORLEVEL%"
) else (
    echo Test executable run_tests.exe not found in build.
    set "RC=1"
)
popd
if %RC% neq 0 (
    echo Tests failed or did not run (code %RC%).
    exit /b %RC%
)
exit /b 0

:report
set "MODULE=%~1"
if "%MODULE%"=="" set "MODULE=general"
if not exist reports mkdir reports
if not exist "reports\%MODULE%" mkdir "reports\%MODULE%"

echo -- Report: generating coverage for module %MODULE%
if not exist build (
    echo Build directory not found. Run "build" first.
    exit /b 1
)
pushd build
where llvm-profdata >nul 2>&1
if %ERRORLEVEL%==0 (
    rem LLVM tooling available
    set "PROFDATA=default.profdata"
    rem Merge any .profraw files if present
    dir /b "*.profraw" >nul 2>&1
    if %ERRORLEVEL%==0 (
        echo Merging .profraw files into %PROFDATA%...
        llvm-profdata merge -sparse *.profraw -o "%PROFDATA%" 2>nul
        if %ERRORLEVEL% neq 0 (
            echo llvm-profdata merge failed with code %ERRORLEVEL%.
        )
    )

    if exist "%PROFDATA%" (
        rem ensure output dir exists (absolute)
        set "OUTDIR=%CD%\..\reports\%MODULE%"
        if not exist "%OUTDIR%" mkdir "%OUTDIR%"
        set "EXEPATH=%CD%\run_tests.exe"
        echo Running llvm-cov to generate HTML into %OUTDIR%
        llvm-cov show "%EXEPATH%" -instr-profile="%PROFDATA%" -format=html -output-dir="%OUTDIR%" --source-root="%CD%\.."
        if %ERRORLEVEL%==0 (
            echo Coverage report created at reports\%MODULE%\index.html
            popd
            exit /b 0
        ) else (
            echo llvm-cov failed with code %ERRORLEVEL%, falling back to gcovr.
        )
    ) else (
        echo No profile data found for llvm-cov; falling back to gcovr.
    )
)

popd
rem Fallback: gcovr report from repo root
echo Generating report with gcovr (fallback)
gcovr -r . --html -o "reports\%MODULE%\coverage.html" --html-details --filter "src/" --root . --gcov-ignore-errors=no_working_dir_found
if %ERRORLEVEL%==0 (
    echo Coverage report for %MODULE% created at reports\%MODULE%\coverage.html
    exit /b 0
) else (
    echo gcovr failed with code %ERRORLEVEL%.
    exit /b %ERRORLEVEL%
)