cmake_minimum_required(VERSION 3.14)
project(CalculatorProject)

set(CMAKE_CXX_STANDARD 17)

# 1. Cấu hình cờ Coverage cho GCC (MinGW)
if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage -fprofile-arcs -ftest-coverage")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
endif()

# 2. Tự động tải Google Test
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/heads/main.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# 3. Quét file nguồn
include_directories(inc)
file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.c")
file(GLOB_RECURSE TEST_SOURCES "test/*.cpp")

# 4. Tạo file thực thi test
add_executable(run_tests ${SOURCES} ${TEST_SOURCES})
# Link with gtest_main and gmock for Google Test/Mock framework
target_link_libraries(run_tests gtest_main gmock_main)