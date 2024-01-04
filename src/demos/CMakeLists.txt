set(VULKAN_TRIANGLE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/vulkan_triangle/main.cpp
)

add_executable(demo_vulkan_triangle ${VULKAN_TRIANGLE_SOURCES})
target_link_libraries(demo_vulkan_triangle
    PUBLIC  gu2::os
    PUBLIC  gu2::vulkan
)
set_property(TARGET demo_vulkan_triangle PROPERTY CXX_STANDARD 20)
