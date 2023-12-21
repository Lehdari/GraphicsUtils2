cmake_minimum_required(VERSION 3.27)
project(GraphicsUtils2)

# Fetch 3rd party dependencies
add_subdirectory(ext)

# Include directory to use for build tree
set(GU2_INCLUDE_DIR_BUILD ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Define installation targets
add_subdirectory(src)
