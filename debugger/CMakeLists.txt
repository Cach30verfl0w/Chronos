cmake_minimum_required(VERSION 3.18)
project(chronos VERSION 1.0.0)

file(GLOB_RECURSE CHRONOS_SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/debugger/src/*.c*")
message("Configure chronos debugger")

# Add application
add_executable(chronos-debugger ${CHRONOS_SOURCE_FILES})
target_include_directories(chronos-debugger PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/debugger/include")
target_link_libraries(chronos-debugger PRIVATE debug-static)

# Add cxxopts to chronos
FetchContent_Declare(
        cxxopts
        GIT_REPOSITORY https://github.com/jarro2783/cxxopts.git
        GIT_TAG master
        GIT_PROGRESS true
)
FetchContent_MakeAvailable(cxxopts)

target_include_directories(chronos-debugger PUBLIC "${CMAKE_BINARY_DIR}/_deps/cxxopts-src/include")

# Add spdlog to chronos
FetchContent_Declare(
        spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG v1.x)
FetchContent_Populate(spdlog)

target_include_directories(chronos-debugger PUBLIC "${CMAKE_BINARY_DIR}/_deps/spdlog-src/include")
add_compile_definitions(SPDLOG_HEADER_ONLY SPDLOG_FMT_EXTERNAL)

if (${BUILD_DEBUG})
    message(STATUS "Verbose logging is enabled in debug build")
    add_compile_definitions(SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE)
else ()
    add_compile_definitions(SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_INFO)
endif ()