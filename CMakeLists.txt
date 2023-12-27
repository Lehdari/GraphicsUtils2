cmake_minimum_required(VERSION 3.27)
project(GraphicsUtils2)

include(cmake/select_backend.cmake)

# Configuration
set(GU2_BACKEND_OPTIONS sdl2 glfw)
set(GU2_BACKEND "SDL2" CACHE STRING "OS interfacing backend")
option(GU2_SHARED_LIBS "Build shared libraries instead of static ones" ON)
option(GU2_ENABLE_TESTS "Build tests" ON)
option(GU2_BUILD_DEMOS "Build demo applications" OFF)

gu2_select_backend(${GU2_BACKEND})

# Fetch 3rd party dependencies
add_subdirectory(ext)

# Include directory to use for build tree
set(GU2_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Define installation targets
add_subdirectory(src)
