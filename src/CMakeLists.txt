# Library sources
set(GU2_OS_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/gu2_os/Window.cpp
)

# Library targets
if (GU2_SHARED_LIBS)
    add_library(gu2_os SHARED ${GU2_OS_SOURCES})
else()
    add_library(gu2_os STATIC ${GU2_OS_SOURCES})
endif()

# Library properties
target_link_libraries(gu2_os
    PUBLIC  SDL2::SDL2
)
target_include_directories(gu2_os
    PUBLIC  $<BUILD_INTERFACE:${GU2_INCLUDE_DIR}> $<INSTALL_INTERFACE:include>
    PRIVATE ${GU2_INCLUDE_DIR}/gu2_os
)
set_property(TARGET gu2_os PROPERTY CXX_STANDARD 17)

# Library aliases
add_library(gu2::os ALIAS gu2_os)

# Demos (TODO: add option for disabling these)
add_subdirectory(demos)
