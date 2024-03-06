cmake_minimum_required(VERSION 3.18)
project(chronos VERSION 1.0.0)

include(FetchContent)
add_compile_definitions(CHRONOS_VERSION="${CMAKE_PROJECT_VERSION}")

# Default C++ and C standard
if (NOT DEFINED CMAKE_CXX_STANDARD)
    message(STATUS "No C++ version specified, defaulting to C++20")
    set(CMAKE_CXX_STANDARD 20)
endif ()

if (NOT DEFINED CMAKE_C_STANDARD)
    message(STATUS "No C version specified, defaulting to C17")
    set(CMAKE_C_STANDARD 17)
endif ()

# Detect architecture
if (CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
    add_compile_definitions(ARCH_X86_64)
elseif (CMAKE_SYSTEM_PROCESSOR STREQUAL "X86")
    add_compile_definitions(ARCH_X86)
else ()
    message(FATAL_ERROR "Unsupported architecture: ${CMAKE_SYSTEM_PROCESSOR}")
endif ()

# Detect compiler
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(COMPILER_CLANG TRUE)
    add_compile_definitions(COMPILER_CLANG)
    message(STATUS "Detected Clang compiler")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(COMPILER_GCC TRUE)
    add_compile_definitions(COMPILER_GCC)
    message(STATUS "Detected GCC compiler")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(COMPILER_MSVC TRUE)
    add_compile_options("/Zc:__cplusplus" "/utf-8")
    add_compile_definitions(COMPILER_MSVC)
    message(STATUS "Detected MSVC compiler")
endif ()

# Detect platform
if (WIN32)
    add_compile_definitions(PLATFORM_WINDOWS)
    link_libraries(kernel32.lib user32.lib)
    message(STATUS "Detected Windows platform")
elseif (APPLE)
    add_compile_definitions(PLATFORM_MACOS PLATFORM_UNIX)
    message(STATUS "Detected MacOS platform")
elseif (UNIX)
    if (NOT APPLE)
        add_compile_definitions(PLATFORM_LINUX)
    endif ()
    add_compile_definitions(PLATFORM_UNIX)
    message(STATUS "Detected Unixoid platform")
else ()
    message(FATAL_ERROR "Unsupported platform '${CMAKE_SYSTEM_NAME}'")
endif ()

# Detect build type
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(BUILD_DEBUG)
    set(BUILD_DEBUG ON)

    # Enable safe STL, disable inlining and optimizations
    if (PLATFORM_LINUX AND COMPILER_GCC)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0 -fno-inline -D_GLIBCXX_DEBUG_BACKTRACE")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -fno-inline -D_GLIBCXX_DEBUG_BACKTRACE")
    endif ()

    # Enable checked iterators
    if (PLATFORM_WINDOWS AND COMPILER_MSVC)
        add_compile_definitions(_ITERATOR_DEBUG_LEVEL=1)
    endif ()
else ()
endif ()

# Detect bit size
if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    add_compile_definitions(CPU_64_BIT)
else ()
    add_compile_definitions(CPU_32_BIT)
endif ()

# ZENDBG as application
file(GLOB_RECURSE SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
add_executable(${CMAKE_PROJECT_NAME} ${SOURCE_FILES})
add_library("${CMAKE_PROJECT_NAME}-static" STATIC ${SOURCE_FILES})
target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_include_directories("${CMAKE_PROJECT_NAME}-static" PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

# Add cxxopts
FetchContent_Declare(
        cxxopts
        GIT_REPOSITORY https://github.com/jarro2783/cxxopts.git
        GIT_TAG master
        GIT_PROGRESS true
)
FetchContent_MakeAvailable(cxxopts)

target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC "${CMAKE_BINARY_DIR}/_deps/cxxopts-src/include")
target_include_directories("${CMAKE_PROJECT_NAME}-static" PUBLIC "${CMAKE_BINARY_DIR}/_deps/cxxopts-src/include")

# Add fmt
FetchContent_Declare(
        fmt
        GIT_REPOSITORY https://github.com/fmtlib/fmt.git
        GIT_TAG master
        GIT_PROGRESS true
)
FetchContent_Populate(fmt)

target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC "${CMAKE_BINARY_DIR}/_deps/fmt-src/include")
target_include_directories("${CMAKE_PROJECT_NAME}-static" PUBLIC "${CMAKE_BINARY_DIR}/_deps/fmt-src/include")
add_compile_definitions(FMT_HEADER_ONLY)

# Add spdlog
FetchContent_Declare(
        spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG v1.x)
FetchContent_Populate(spdlog)

target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC "${CMAKE_BINARY_DIR}/_deps/spdlog-src/include")
target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC "${CMAKE_BINARY_DIR}/_deps/spdlog-src/include")
add_compile_definitions(SPDLOG_HEADER_ONLY SPDLOG_FMT_EXTERNAL)

if (${BUILD_DEBUG})
    message(STATUS "Verbose logging is enabled in debug build")
    add_compile_definitions(SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE)
else ()
    add_compile_definitions(SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_INFO)
endif ()

# Add kstd-core
FetchContent_Declare(
        kstd-core
        GIT_REPOSITORY https://github.com/karmakrafts/kstd-core.git
        GIT_TAG master
        GIT_PROGRESS true
)
FetchContent_Populate(kstd-core)

target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC "${CMAKE_BINARY_DIR}/_deps/kstd-core-src/include")
target_include_directories("${CMAKE_PROJECT_NAME}-static" PUBLIC "${CMAKE_BINARY_DIR}/_deps/kstd-core-src/include")