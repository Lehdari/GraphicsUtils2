# vulkan_triangle demo
set(VULKAN_TRIANGLE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/vulkan_triangle/main.cpp
)

add_executable(demo_vulkan_triangle ${VULKAN_TRIANGLE_SOURCES})
target_link_libraries(demo_vulkan_triangle
    PUBLIC  gu2::os
    PUBLIC  gu2::vulkan
    PUBLIC  gu2::util
)
set_property(TARGET demo_vulkan_triangle PROPERTY CXX_STANDARD 20)

# vulkan_box demo
set(VULKAN_BOX_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/vulkan_box/main.cpp
)

add_executable(demo_vulkan_box ${VULKAN_BOX_SOURCES})
target_link_libraries(demo_vulkan_box
    PUBLIC  gu2::os
    PUBLIC  gu2::vulkan
    PUBLIC  gu2::util
)
target_compile_definitions(demo_vulkan_box
    PUBLIC  -DASSETS_DIR=\"${GU2_ASSETS_DIR}\"
)
set_property(TARGET demo_vulkan_box PROPERTY CXX_STANDARD 20)
