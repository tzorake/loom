cmake_minimum_required(VERSION 3.21)

# ── loom-rhi ─────────────────────────────────────────────────────────────────
#
# Rendering Hardware Interface — OpenGL 3.3 Core (desktop) and WebGL 2.0 (WASM).
#
# Usage:
#   cmake -DLOOM_BUILD_RHI=ON ...
#
# The library exposes its public headers via the loom-rhi target and links
# against loom-core so callers only need to link loom-rhi.

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

# ── Backend selection ─────────────────────────────────────────────────────────

if(CMAKE_SYSTEM_NAME STREQUAL "WASI" OR LOOM_PLATFORM_WEB)
    # WebGL 2.0 backend — all GL calls are WASM imports.
    target_sources(loom-rhi PRIVATE
        src/webgl/tzrhiwebgl.cpp
    )
    target_compile_definitions(loom-rhi PRIVATE LOOM_RHI_WEBGL)

else()
    # OpenGL 3.3 Core backend.
    target_sources(loom-rhi PRIVATE
        src/opengl/tzrhiopengl.cpp
        src/opengl/tzglcontext.hpp   # header-only for private includes
    )
    target_compile_definitions(loom-rhi PRIVATE LOOM_RHI_OPENGL)

    if(APPLE)
        target_sources(loom-rhi PRIVATE src/opengl/tzglcontext_cocoa.mm)
        target_link_libraries(loom-rhi PUBLIC
            "-framework OpenGL"
            "-framework Cocoa"
        )

    elseif(WIN32)
        target_sources(loom-rhi PRIVATE src/opengl/tzglcontext_win32.cpp)
        target_link_libraries(loom-rhi PUBLIC opengl32)

    else()
        # Linux — X11 or Wayland (mirror the detection in loom-core).
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

# ── Examples ─────────────────────────────────────────────────────────────────
option(LOOM_BUILD_RHI_EXAMPLES "Build loom-rhi example programs" ${LOOM_BUILD_RHI})
if(LOOM_BUILD_RHI_EXAMPLES)
    add_subdirectory(examples/quad)
endif()
