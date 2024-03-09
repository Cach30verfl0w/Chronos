cmake_minimum_required(VERSION 3.18)
project(libdebug VERSION 1.0.0)

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
