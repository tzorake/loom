cmake_minimum_required(VERSION 3.21)

add_library(loom-rhi
    src/tzrhi.cpp
    src/tzrhiresource.cpp
    src/tzshader.cpp
)

target_include_directories(loom-rhi
    PUBLIC  include
    PRIVATE src
)

target_link_libraries(loom-rhi PUBLIC loom-core)

if(CMAKE_SYSTEM_NAME STREQUAL "WASI" OR LOOM_PLATFORM_WEB)
    target_sources(loom-rhi PRIVATE
        src/webgl/tzrhiwebgl.cpp
    )
    target_compile_definitions(loom-rhi PUBLIC LOOM_RHI_WEBGL)

else()
    target_sources(loom-rhi PRIVATE
        src/opengl/tzrhiopengl.cpp
        src/opengl/tzglcontext.hpp   # header-only for private includes
    )
    target_compile_definitions(loom-rhi PUBLIC LOOM_RHI_OPENGL)

    if(APPLE)
        target_sources(loom-rhi PRIVATE src/opengl/tzglcontext_cocoa.cpp)
        target_compile_definitions(loom-rhi PRIVATE GL_SILENCE_DEPRECATION)
        target_link_libraries(loom-rhi PUBLIC
            "-framework OpenGL"
            "-framework Cocoa"
        )

    elseif(WIN32)
        target_sources(loom-rhi PRIVATE src/opengl/tzglcontext_win32.cpp)
        target_link_libraries(loom-rhi PUBLIC opengl32)

    else()
        if(LOOM_BACKEND_X11)
            target_sources(loom-rhi PRIVATE src/opengl/tzglcontext_x11.cpp)
            target_link_libraries(loom-rhi PUBLIC GL X11)
        else()
            target_sources(loom-rhi PRIVATE src/opengl/tzglcontext_wayland.cpp)
            target_link_libraries(loom-rhi PUBLIC GL EGL wayland-client wayland-egl)
        endif()
    endif()
endif()

# Export include path for consumers that add this as a subproject.
target_include_directories(loom-rhi INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

if(LOOM_BUILD_EXAMPLES)
    add_subdirectory(examples/quad)
endif()
