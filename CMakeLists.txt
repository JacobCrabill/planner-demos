cmake_minimum_required(VERSION 3.10)

# set the project name
project(PlannerDemo VERSION 0.1)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
    if(${CMAKE_CXX_COMPILER_VERSION} VERSION_LESS 8.1)
        message("c++ compiler is: ${CMAKE_CXX_COMPILER_ID}")
        message("c++ compiler version is: ${CMAKE_CXX_COMPILER_VERSION}")
        add_definitions(-DFORCE_EXPERIMENTAL_FS)
    endif()
else()
    message(WARNING "Non-GNU compilers not supported - compile at your own risk.")
endif()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-fPIC -Wall -Wextra -Wno-conversion -Wno-unknown-pragmas")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "-Ofast")

option(ENABLE_PROFILING "Enable profiling of app - results saved to json." OFF)
if(ENABLE_PROFILING)
    add_definitions(-DINCLUDE_PROFILING)
endif()

option(ENABLE_LIBNOISE "Enable use of libnoise for procedurally-generated terrain" ON)

if(ENABLE_LIBNOISE)
    add_definitions(-DENABLE_LIBNOISE)
endif()

include_directories(${PlannerDemo_SOURCE_DIR}/include/)
include_directories(${PlannerDemo_SOURCE_DIR}/3rdparty/)
include_directories(${PlannerDemo_SOURCE_DIR}/3rdparty/libnoise/include/)
include_directories(${PlannerDemo_SOURCE_DIR}/3rdparty/yaml-cpp/include/)

add_executable(planner-demo
    src/astar.cpp
    src/gamemap.cpp
    src/main.cpp
    src/plannerDemo.cpp
    src/util.cpp
    src/tileset.cpp
)

# Build 3rd-party modules as static libraries
set(YAML_CPP_BUILD_CONTRIB OFF CACHE BOOL "Turn off extra stuff" FORCE)
set(YAML_CPP_BUILD_TOOLS   OFF CACHE BOOL "Turn off extra stuff" FORCE)
set(YAML_CPP_BUILD_TESTS   OFF CACHE BOOL "Turn off extra stuff" FORCE)
add_subdirectory(3rdparty/yaml-cpp)
if(ENABLE_LIBNOISE)
    add_subdirectory(3rdparty/libnoise)
endif()

add_custom_command(TARGET planner-demo POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/resources ${CMAKE_BINARY_DIR}/resources
)

set(THREADS_PREFER_PTHREAD_FLAG ON)
set(OpenGL_GL_PREFERENCE LEGACY)

find_package(Threads REQUIRED)
find_package(OpenGL REQUIRED)
find_package(PNG REQUIRED)
find_package(X11 REQUIRED)

target_link_libraries(planner-demo OpenGL::OpenGL)
target_link_libraries(planner-demo OpenGL::GLX)
target_link_libraries(planner-demo Threads::Threads)
target_link_libraries(planner-demo ${PNG_LIBRARIES})
target_link_libraries(planner-demo ${X11_LIBRARIES})
target_link_libraries(planner-demo "stdc++fs")
#  ^- TODO: std::filesystem included by default in GCC > 9?

target_link_libraries(planner-demo yaml-cpp)
if(ENABLE_LIBNOISE)
    target_link_libraries(planner-demo noise)
endif()
