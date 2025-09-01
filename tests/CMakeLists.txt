# Test CMakeLists.txt - comprehensive unit tests for olshell
cmake_minimum_required(VERSION 3.20)

# Find and include Google Test
find_package(GTest REQUIRED)
if(NOT GTest_FOUND)
    # Download and build Google Test if not found
    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50e33f9cfd640a44b5b44905c2add0.zip
    )
    FetchContent_MakeAvailable(googletest)
endif()

# Include test directories
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/src)

# Create test executables
set(TEST_SOURCES
    test_main.cpp
    test_shell.cpp
    test_parser.cpp
    test_executor.cpp
    test_builtins.cpp
    test_utils.cpp
    test_config.cpp
    test_autocomplete.cpp
    test_input_manager.cpp
)

# Create main test executable
add_executable(olshell_tests ${TEST_SOURCES})

# Link test libraries and olshell components
target_link_libraries(olshell_tests 
    gtest 
    gtest_main
    # Link individual source files since we don't have a static library
    ${CMAKE_SOURCE_DIR}/src/shell.cpp
    ${CMAKE_SOURCE_DIR}/src/parser/parser.cpp
    ${CMAKE_SOURCE_DIR}/src/executor/executor.cpp
    ${CMAKE_SOURCE_DIR}/src/builtins/alias.cpp
    ${CMAKE_SOURCE_DIR}/src/builtins/cat.cpp
    ${CMAKE_SOURCE_DIR}/src/builtins/cd.cpp
    ${CMAKE_SOURCE_DIR}/src/builtins/clear.cpp
    ${CMAKE_SOURCE_DIR}/src/builtins/echo.cpp
    ${CMAKE_SOURCE_DIR}/src/builtins/history.cpp
    ${CMAKE_SOURCE_DIR}/src/builtins/ls.cpp
    ${CMAKE_SOURCE_DIR}/src/builtins/pwd.cpp
    ${CMAKE_SOURCE_DIR}/src/builtins/rm.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/autocomplete.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/colors.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/config.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/fs.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/input_manager.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/linenoise.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/script.cpp
)

# Enable testing
enable_testing()

# Add test discovery
include(GoogleTest)
gtest_discover_tests(olshell_tests)

# Create individual test targets for CI
add_test(NAME shell_tests COMMAND olshell_tests --gtest_filter="ShellTest.*")
add_test(NAME parser_tests COMMAND olshell_tests --gtest_filter="ParserTest.*")
add_test(NAME executor_tests COMMAND olshell_tests --gtest_filter="ExecutorTest.*")
add_test(NAME builtin_tests COMMAND olshell_tests --gtest_filter="BuiltinTest.*")
add_test(NAME config_tests COMMAND olshell_tests --gtest_filter="ConfigTest.*")
add_test(NAME utils_tests COMMAND olshell_tests --gtest_filter="UtilsTest.*")
