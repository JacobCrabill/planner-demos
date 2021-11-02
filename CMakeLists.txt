cmake_minimum_required(VERSION 3.10)

# set the project name
project(PlannerDemo VERSION 0.1)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-fPIC -Wall -Wextra -Wconversion -Wno-unknown-pragmas")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "-Ofast")

if(ENABLE_PROFILING)
    add_definitions(-DINCLUDE_PROFILING)
endif()

option(ENABLE_LIBNOISE "Enable use of libnoise for procedurally-generated terrain" True)

if(ENABLE_LIBNOISE)
    add_definitions(-DENABLE_LIBNOISE)
endif()

include_directories(${PlannerDemo_SOURCE_DIR}/include/)
include_directories(${PlannerDemo_SOURCE_DIR}/3rdparty/)
include_directories(${PlannerDemo_SOURCE_DIR}/3rdparty/libnoise/include/)

add_executable(planner-demo
    src/astar.cpp
    src/gamemap.cpp
    src/main.cpp
    src/util.cpp
)

# Build pugixml as a static lib
add_subdirectory(3rdparty/pugixml)
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

target_link_libraries(planner-demo pugixml-static)
if(ENABLE_LIBNOISE)
    target_link_libraries(planner-demo noise)
endif()
