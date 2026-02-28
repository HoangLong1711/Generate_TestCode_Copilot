cmake_minimum_required(VERSION 3.14)
project(GoogleTestCPlus CXX)

set(CMAKE_CXX_STANDARD 17)

# 1. Configure Coverage flags for GCC (MinGW) and Clang
option(USE_LLVM_COVERAGE "Use LLVM source-based coverage (llvm-cov / llvm-profdata)" ON)

if(MSVC)
  # MSVC compiler
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND USE_LLVM_COVERAGE)
  # Clang with LLVM coverage: use source-based coverage instrumentation
  add_compile_options(-fprofile-instr-generate -fcoverage-mapping)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-instr-generate")
elseif(CMAKE_COMPILER_IS_GNUCXX OR (CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND NOT USE_LLVM_COVERAGE))
  # GCC (or Clang when not using LLVM coverage): fallback to gcov-style flags
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage -fprofile-arcs -ftest-coverage")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
endif()

# 2. Automatically download Google Test
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/heads/main.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# 3. Scan source files
include_directories(inc)
file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.c")
file(GLOB_RECURSE TEST_SOURCES "test/*.cpp")

# 4. Create a test executable file.
add_executable(run_tests ${SOURCES} ${TEST_SOURCES})
# Link with gtest_main and gmock for Google Test/Mock framework
target_link_libraries(run_tests gtest_main gmock_main)