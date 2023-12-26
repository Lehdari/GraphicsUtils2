# Library sources
set(GU2_OS_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/gu2_os/EventHandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gu2_os/${GU2_BACKEND_SOURCE_FILE}
)

# Library targets
if (GU2_SHARED_LIBS)
    add_library(gu2_os SHARED ${GU2_OS_SOURCES})
else()
    add_library(gu2_os STATIC ${GU2_OS_SOURCES})
endif()

# Library properties
target_link_libraries(gu2_os
    PUBLIC  ${GU2_BACKEND_LIB}
)
target_include_directories(gu2_os
    PUBLIC  $<BUILD_INTERFACE:${GU2_INCLUDE_DIR}> $<INSTALL_INTERFACE:include>
    PRIVATE ${GU2_INCLUDE_DIR}/gu2_os
)
target_compile_definitions(gu2_os
    PUBLIC  -DGU2_BACKEND=${GU2_BACKEND_ENUM}
    PUBLIC  -DGU2_BACKEND_NAME=\"${GU2_BACKEND}\"
)
set_property(TARGET gu2_os PROPERTY CXX_STANDARD 20)

# Library aliases
add_library(gu2::os ALIAS gu2_os)

# Demos (TODO: add option for disabling these)
add_subdirectory(demos)
