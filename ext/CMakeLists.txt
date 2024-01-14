add_subdirectory(SDL)
add_subdirectory(glfw)

# stb added as an interface library
add_library(stb INTERFACE)
target_include_directories(stb
    INTERFACE   ${CMAKE_CURRENT_SOURCE_DIR}/stb
)
