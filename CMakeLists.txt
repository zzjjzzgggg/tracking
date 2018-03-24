cmake_minimum_required(VERSION 3.5.1)
project (Discoverability C CXX)

include(CheckCXXCompilerFlag)

check_cxx_compiler_flag("-std=c++1z" COMPILER_SUPPORTS_CXX_1z)
check_cxx_compiler_flag("-std=c++14" COMPILER_SUPPORTS_CXX_14)
check_cxx_compiler_flag("-std=c++17" COMPILER_SUPPORTS_CXX_17)

if (COMPILER_SUPPORTS_CXX_1z)
  message(STATUS "Compiling with C++1z")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z")
elseif (COMPILER_SUPPORTS_CXX_14)
  message(STATUS "Compiling with C++14")
  set(CMAKE_CXX_STANDARD 14)
else()
  message(FATAL_ERROR "Please install a modern C++ compiler.")
endif()

set(CMAKE_BUILD_TYPE Release)
# set(CMAKE_CXX_COMPILER=/usr/bin/g++)
# set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-pthread")

set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} $ENV{GIT_REPO}/misc/cmake)
message(STATUS "CMAKE_MODULE_PATH: " ${CMAKE_MODULE_PATH})

# required libraries from system
find_package(Threads REQUIRED)

# gflags
find_package(gflags REQUIRED)

# cpplib
find_package(CPPLIB REQUIRED)
if (NOT CPPLIB_FOUND)
  message(FATAL_ERROR "Package CPPLIB not found")
endif ()
add_definitions(${CPPLIB_DEFINITIONS})
include_directories(${CPPLIB_INCLUDE_DIRS})

# libraries
set(LIBRARIES ${LIBRARIES} gflags ${CPPLIB_LIBRARIES})

# MESSAGE(STATUS "LIBRARIES: " ${LIBRARIES})

add_subdirectory(src)
