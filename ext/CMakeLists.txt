add_subdirectory(eigen)
add_subdirectory(glfw)
add_subdirectory(SDL)

# stb added as an interface library
add_library(stb INTERFACE)
target_include_directories(stb
    INTERFACE   ${CMAKE_CURRENT_SOURCE_DIR}/stb
)
