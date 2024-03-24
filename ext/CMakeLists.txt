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

# shaderc
execute_process(COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/shaderc/utils/git-sync-deps)
set(SHADERC_SKIP_TESTS ON)
add_subdirectory(shaderc)

#SPIRV-Reflect
add_library(spirv-reflect-static STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/SPIRV-Reflect/spirv_reflect.h
    ${CMAKE_CURRENT_SOURCE_DIR}/SPIRV-Reflect/spirv_reflect.cpp
)
target_include_directories(spirv-reflect-static
    PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/SPIRV-Reflect
)

# stb
add_library(stb INTERFACE)
target_include_directories(stb
    INTERFACE   ${CMAKE_CURRENT_SOURCE_DIR}/stb
)

# nlohmann's json
add_subdirectory(json)
