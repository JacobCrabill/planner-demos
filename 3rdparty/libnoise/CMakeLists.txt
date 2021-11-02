cmake_minimum_required(VERSION 3.13)

cmake_policy(VERSION 3.2)
cmake_policy(SET CMP0076 NEW)

# set the project name
project(LibNoise VERSION 0.9.0)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-fPIC -Wall -Wextra -Wconversion -Wno-unused-parameter")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O2")
set(CMAKE_CXX_FLAGS_RELEASE "-Ofast")

include_directories(${LibNoise_SOURCE_DIR}/include/noise/)
include_directories(${LibNoise_SOURCE_DIR}/include/noise/model)
include_directories(${LibNoise_SOURCE_DIR}/include/noise/module)

add_library(noise STATIC)

add_subdirectory(src)
