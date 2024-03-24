cmake_minimum_required(VERSION 3.18)
project(chronos VERSION 1.0.0)

option(LIBDEBUG_COMPILE_TESTS "Compile tests for LibDebug" ON)
option(CHRONOS_USE_MOLD "Use mold for all modules when available" ON)

# Use Mold
if (LIBDEBUG_USE_MOLD)
    find_program(CMX_MOLD_EXECUTABLE "mold")
    if (CMX_MOLD_EXECUTABLE AND NOT COMPILER_CLANG)
        message(STATUS "Detected mold linker, substituting")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fuse-ld=mold")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fuse-ld=mold")

        if (NOT DEFINED CMX_NUM_LINK_THREADS) # So we can pass this in via CLI
            cmake_host_system_information(RESULT CMX_NUM_LINK_THREADS QUERY NUMBER_OF_LOGICAL_CORES)
        endif () # NOT DEFINED NUM_LINK_THREADS

        message(STATUS "Using ${CMX_NUM_LINK_THREADS} threads for linking")
        add_link_options("LINKER:--threads,--thread-count=${CMX_NUM_LINK_THREADS}")
    endif () # MOLD_EXECUTABLE AND NOT COMPILER_CLANG
endif () # LIBDEBUG_USE_MOLD

# Include CMake Modules
include(FetchContent)
include(TestBigEndian)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake;")
include(detect)
include(${CMAKE_CURRENT_SOURCE_DIR}/libdebug/CMakeLists.txt)
include(${CMAKE_CURRENT_SOURCE_DIR}/debugger/CMakeLists.txt)