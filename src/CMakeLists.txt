# gu2::util
add_library(gu2_util INTERFACE)
target_link_libraries(gu2_util
    INTERFACE   Eigen3::Eigen
)
target_include_directories(gu2_util
    INTERFACE   $<BUILD_INTERFACE:${GU2_INCLUDE_DIR}> $<INSTALL_INTERFACE:include>
    #PRIVATE     ${GU2_INCLUDE_DIR}/gu2_util
)
set_property(TARGET ${libname} PROPERTY CXX_STANDARD 20)
add_library(gu2::util ALIAS gu2_util)


# gu2::os
macro(gu2_add_os_lib libname) # macro for generating versions for different backends
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

gu2_add_os_lib(gu2_os)
add_library(gu2::os ALIAS gu2_os)


# gu2::vulkan
macro(gu2_add_vulkan_lib libname) # macro for generating versions for different backends
    # Library sources
    set(GU2_VULKAN_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/gu2_vulkan/${GU2_BACKEND_SOURCE_FILE}
    )

    # Library targets
    if (GU2_SHARED_LIBS)
        add_library(${libname} SHARED ${GU2_VULKAN_SOURCES})
    else()
        add_library(${libname} STATIC ${GU2_VULKAN_SOURCES})
    endif()

    # Library properties
    target_link_libraries(${libname}
        PUBLIC  gu2::os
        PUBLIC  vulkan
    )
    target_include_directories(${libname}
        PUBLIC  $<BUILD_INTERFACE:${GU2_INCLUDE_DIR}> $<INSTALL_INTERFACE:include>
        PRIVATE ${GU2_INCLUDE_DIR}/gu2_vulkan
    )
    target_compile_definitions(${libname}
        PUBLIC  -DGU2_BACKEND=${GU2_BACKEND_ENUM}
        PUBLIC  -DGU2_BACKEND_NAME=\"${GU2_BACKEND}\"
    )
    set_property(TARGET ${libname} PROPERTY CXX_STANDARD 20)
    add_custom_command(
        TARGET ${libname} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${GU2_SHADER_DIR} $<TARGET_FILE_DIR:${libname}>/shader
        COMMENT "Copying shaders" VERBATIM
    )
endmacro()

gu2_add_vulkan_lib(gu2_vulkan)
add_library(gu2::vulkan ALIAS gu2_vulkan)


# Tests
if(${GU2_ENABLE_TESTS})
    # build versions of gu2_os with every backend for tests
    foreach(backend ${GU2_BACKEND_OPTIONS})
        gu2_select_backend(${backend} OFF)
        gu2_add_os_lib(gu2_os_${backend})
        gu2_add_vulkan_lib(gu2_vulkan_${backend})
    endforeach()
    add_subdirectory(tests)
endif()


# Demos (TODO: add option for disabling these)
add_subdirectory(demos)
