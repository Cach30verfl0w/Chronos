cmake_minimum_required(VERSION 3.18)
project(libdebug VERSION 1.0.0)

# Acquire source code
file(GLOB_RECURSE LIBDEBUG_SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/libdebug/src/*.c*")
message("Configure libdebug debugging library")

# Add libraries
message(STATUS "Configure libdebug shared library")
add_library(debug SHARED ${LIBDEBUG_SOURCE_FILES})
target_include_directories(debug PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/libdebug/include")

message(STATUS "Configure libdebug static library")
add_library(debug-static STATIC ${LIBDEBUG_SOURCE_FILES})
target_include_directories(debug-static PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/libdebug/include")

# Add fmt to libdebug
FetchContent_Declare(
        fmt
        GIT_REPOSITORY https://github.com/fmtlib/fmt.git
        GIT_TAG master
        GIT_PROGRESS true
)
FetchContent_Populate(fmt)

target_include_directories(debug PUBLIC "${CMAKE_BINARY_DIR}/_deps/fmt-src/include")
target_include_directories(debug-static PUBLIC "${CMAKE_BINARY_DIR}/_deps/fmt-src/include")
add_compile_definitions(FMT_HEADER_ONLY)

# Add kstd-core to libdebug
FetchContent_Declare(
        kstd-core
        GIT_REPOSITORY https://github.com/karmakrafts/kstd-core.git
        GIT_TAG master
        GIT_PROGRESS true
)
FetchContent_Populate(kstd-core)

target_include_directories(debug PUBLIC "${CMAKE_BINARY_DIR}/_deps/kstd-core-src/include")
target_include_directories(debug-static PUBLIC "${CMAKE_BINARY_DIR}/_deps/kstd-core-src/include")

# Add gtest to libdebug-test
message("Configure google test for libdebug")
FetchContent_Declare(
        google-test
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG main
)
FetchContent_MakeAvailable(google-test)
enable_testing(TRUE)
include(GoogleTest)

# Add tests
file(GLOB_RECURSE TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/libdebug/tests/*.c*")
add_executable(libdebug-tests ${TEST_SOURCES})
gtest_discover_tests(libdebug-tests)
target_link_libraries(libdebug-tests PUBLIC gtest_main)

target_link_libraries(libdebug-tests PRIVATE debug-static)
add_dependencies(libdebug-tests debug-static)

# Configure samples directory
message("Configure samples directory for libdebug-tests")
file(GLOB SAMPLE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/libdebug/samples/*.c*")
foreach (SAMPLE_FILE ${SAMPLE_FILES})
    get_filename_component(SAMPLE_NAME "${SAMPLE_FILE}" NAME_WE)
    message(STATUS "Configure sample ${SAMPLE_NAME}")
    add_executable(${SAMPLE_NAME} "${SAMPLE_FILE}")
    set_target_properties(${SAMPLE_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/libdebug/samples")
    set_target_properties(${SAMPLE_NAME} PROPERTIES SUFFIX ".out")
    add_dependencies(libdebug-tests ${SAMPLE_NAME})

    # Add macro reference to file
    string(TOUPPER ${SAMPLE_NAME} UPPER_SAMPLE_NAME)
    add_compile_definitions(SAMPLE_${UPPER_SAMPLE_NAME}_FILE="${CMAKE_CURRENT_SOURCE_DIR}/libdebug/samples/${SAMPLE_NAME}.out")
endforeach ()