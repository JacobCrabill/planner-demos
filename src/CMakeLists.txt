
include_directories(${PlannerDemo_SOURCE_DIR}/include/)

# add the executable
add_executable(planner-demo
    astar.cpp
    gamemap.cpp
    main.cpp
)

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

# ==== Alternative: Simple Makefile-style library linking ====
# target_link_libraries(planner-demo "X11")
# target_link_libraries(planner-demo "GL")
# target_link_libraries(planner-demo "pthread")
# target_link_libraries(planner-demo "png")
