target_sources(noise PRIVATE
    latlon.cpp
    noisegen.cpp
)

add_subdirectory(model)
add_subdirectory(module)
