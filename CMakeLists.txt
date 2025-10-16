cmake_minimum_required(VERSION 3.15)
project(flexible-barrier C)

# Set C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Option to enable/disable tests
option(ENABLE_TESTS "Enable building and running tests" ON)

find_package(Threads REQUIRED)

# Include headers
include_directories(${PROJECT_SOURCE_DIR}/include)

# Define the main library
add_library(flexible-barrier src/flexible_barrier.c)

target_link_libraries(flexible-barrier PUBLIC pthread)

# --- Tests section ---
if(ENABLE_TESTS)
    message(STATUS "Building tests is ENABLED")

    # Enable CTest
    include(CTest)
    enable_testing()

    # Create test executable
    add_executable(test1 tests/test1.c)

    # Link with main library
    target_link_libraries(test1 pthread flexible-barrier)

    # Create test executable
    add_executable(test2 tests/test2.c)

    # Link with main library
    target_link_libraries(test2 pthread flexible-barrier)

    # Create test executable
    add_executable(test3 tests/test3.c)

    # Link with main library
    target_link_libraries(test3 pthread flexible-barrier)

    # Create test executable
    add_executable(test4 tests/test4.c)

    # Link with main library
    target_link_libraries(test4 pthread flexible-barrier)

    # Create test executable
    add_executable(test5 tests/test5.c)

    # Link with main library
    target_link_libraries(test5 pthread flexible-barrier)

    # Register test
    add_test(NAME test1 COMMAND test1)
    add_test(NAME test2 COMMAND test2)
    add_test(NAME test3 COMMAND test3)
    add_test(NAME test4 COMMAND test4)
    add_test(NAME test5 COMMAND test5)
else()
    message(STATUS "Building tests is DISABLED")
endif()
