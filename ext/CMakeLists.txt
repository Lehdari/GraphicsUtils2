# Eigen
add_subdirectory(eigen)

# GLFW
if (GU2_SHARED_LIBS)
    set(GLFW_LIBRARY_TYPE "SHARED")
else()
    set(GLFW_LIBRARY_TYPE "STATIC")
endif()
add_subdirectory(glfw)

# SDL
if (GU2_SHARED_LIBS)
    set(SDL_SHARED_ENABLED_BY_DEFAULT ON)
    set(SDL_STATIC_ENABLED_BY_DEFAULT OFF)
endif()
add_subdirectory(SDL)

# stb
add_library(stb INTERFACE)
target_include_directories(stb
    INTERFACE   ${CMAKE_CURRENT_SOURCE_DIR}/stb
)
