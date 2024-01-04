# gu2::os
macro(add_gu2_os_lib libname)
    # Library sources
    set(GU2_OS_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/gu2_os/App.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/gu2_os/Event.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/gu2_os/${GU2_BACKEND_SOURCE_FILE}
    )

    # Library targets
    if (GU2_SHARED_LIBS)
        add_library(${libname} SHARED ${GU2_OS_SOURCES})
    else()
        add_library(${libname} STATIC ${GU2_OS_SOURCES})
    endif()

    # Library properties
    target_link_libraries(${libname}
        PUBLIC  ${GU2_BACKEND_LIB}
    )
    target_include_directories(${libname}
        PUBLIC  $<BUILD_INTERFACE:${GU2_INCLUDE_DIR}> $<INSTALL_INTERFACE:include>
        PRIVATE ${GU2_INCLUDE_DIR}/gu2_os
    )
    target_compile_definitions(${libname}
        PUBLIC  -DGU2_BACKEND=${GU2_BACKEND_ENUM}
        PUBLIC  -DGU2_BACKEND_NAME=\"${GU2_BACKEND}\"
    )
    set_property(TARGET ${libname} PROPERTY CXX_STANDARD 20)
endmacro()

add_gu2_os_lib(gu2_os)
add_library(gu2::os ALIAS gu2_os)


# gu2::vulkan
add_library(gu2_vulkan INTERFACE)
target_link_libraries(gu2_vulkan
    INTERFACE   vulkan
)
target_include_directories(gu2_vulkan
    INTERFACE   $<BUILD_INTERFACE:${GU2_INCLUDE_DIR}> $<INSTALL_INTERFACE:include>
    INTERFACE   ${GU2_INCLUDE_DIR}/gu2_vulkan
)
add_library(gu2::vulkan ALIAS gu2_vulkan)


# Tests
if(${GU2_ENABLE_TESTS})
    # build versions of gu2_os with every backend for tests
    foreach(backend ${GU2_BACKEND_OPTIONS})
        gu2_select_backend(${backend} OFF)
        add_gu2_os_lib(gu2_os_${backend})
    endforeach()
    add_subdirectory(tests)
endif()


# Demos (TODO: add option for disabling these)
add_subdirectory(demos)
