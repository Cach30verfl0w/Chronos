cmake_minimum_required(VERSION 3.18)
project(chronos VERSION 1.0.0)

# Include CMake Modules
include(FetchContent)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake;")
include(detect)
include(${CMAKE_CURRENT_SOURCE_DIR}/libdebug/CMakeLists.txt)
include(${CMAKE_CURRENT_SOURCE_DIR}/debugger/CMakeLists.txt)