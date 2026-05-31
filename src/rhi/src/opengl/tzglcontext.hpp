#pragma once

#include <loom/tzplatformsurface.hpp>

// Platform-agnostic OpenGL context wrapper.
//
// Each platform provides its own implementation file:
//   tzglcontext_cocoa.mm    — NSOpenGLContext (macOS)
//   tzglcontext_win32.cpp   — wglCreateContextAttribsARB (Windows)
//   tzglcontext_x11.cpp     — glXCreateContextAttribsARB (Linux/X11)
//   tzglcontext_wayland.cpp — EGL (Linux/Wayland)
//
// Usage:
//   TzGLContext ctx;
//   ctx.create(nativeHandle, width, height);
//   ctx.makeCurrent();
//   // ... render ...
//   ctx.swapBuffers();
//   ctx.destroy();

class TzGLContext
{
public:
    TzGLContext();
    ~TzGLContext();

    TzGLContext(const TzGLContext &) = delete;
    TzGLContext &operator=(const TzGLContext &) = delete;

    // Create the native GL context and attach it to the window described by
    // handle. w and h are the initial surface dimensions in pixels.
    bool create(const TzNativeWindowHandle &handle, int w, int h);

    // Resize the underlying surface (EGL / WGL pixel format change etc.).
    // On most platforms this is a no-op; on EGL it recreates the surface.
    void resize(int w, int h);

    // Make this context current on the calling thread.
    void makeCurrent();

    // Release the context from the calling thread.
    void doneCurrent();

    // Present the back buffer.
    void swapBuffers();

    // Destroy the context and release all native resources.
    void destroy();

    bool isValid() const { return m_valid; }

private:
    bool m_valid{false};

    // Platform-specific storage — defined per implementation file.
    struct NativeData;
    NativeData *m_native{nullptr};
};
