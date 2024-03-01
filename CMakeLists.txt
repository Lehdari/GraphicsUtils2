cmake_minimum_required(VERSION 3.27)
project(GraphicsUtils2)

include(cmake/select_backend.cmake)

# Configuration
set(GU2_BACKEND_OPTIONS sdl2 glfw)
set(GU2_BACKEND "SDL2" CACHE STRING "OS interfacing backend")
option(GU2_SHARED_LIBS "Build shared libraries instead of static ones" ON)
option(GU2_BUILD_TESTS "Build tests" OFF)
option(GU2_BUILD_DEMOS "Build demo applications" OFF)

gu2_select_backend(${GU2_BACKEND})
if (GU2_SHARED_LIBS)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

# Fetch 3rd party dependencies
add_subdirectory(ext)

find_package(Vulkan REQUIRED)
add_library(vulkan INTERFACE)
target_include_directories(vulkan SYSTEM INTERFACE ${Vulkan_INCLUDE_DIR})
target_link_libraries(vulkan INTERFACE ${Vulkan_LIBRARY})

find_package(OpenMP)

# Required for shader installation
set(GU2_SHADER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shader)

# Include directory to use for build tree
set(GU2_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(GU2_ASSETS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/assets)

# Define installation targets
add_subdirectory(src)
