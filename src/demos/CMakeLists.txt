set(VULKAN_TRIANGLE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/vulkan_triangle/main.cpp
)

add_executable(demo_vulkan_triangle ${VULKAN_TRIANGLE_SOURCES})
