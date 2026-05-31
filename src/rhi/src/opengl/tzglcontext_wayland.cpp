#include "tzglcontext.hpp"

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <wayland-egl.h>

struct TzGLContext::NativeData
{
    EGLDisplay   eglDisplay{EGL_NO_DISPLAY};
    EGLContext   eglContext{EGL_NO_CONTEXT};
    EGLSurface   eglSurface{EGL_NO_SURFACE};
    EGLConfig    eglConfig{nullptr};
    wl_egl_window *eglWindow{nullptr};
    struct wl_surface *wlSurface{nullptr};
    int width{0};
    int height{0};
};

TzGLContext::TzGLContext()
    : m_native(new NativeData)
{}

TzGLContext::~TzGLContext()
{
    destroy();
    delete m_native;
}

bool TzGLContext::create(const TzNativeWindowHandle &handle, int w, int h)
{
    struct wl_display *wlDisplay = static_cast<struct wl_display *>(handle.wlDisplay);
    m_native->wlSurface = static_cast<struct wl_surface *>(handle.wlSurface);
    m_native->width  = w;
    m_native->height = h;

    m_native->eglDisplay = eglGetDisplay(static_cast<EGLNativeDisplayType>(wlDisplay));
    if (m_native->eglDisplay == EGL_NO_DISPLAY)
        return false;

    EGLint major = 0, minor = 0;
    if (!eglInitialize(m_native->eglDisplay, &major, &minor))
        return false;

    eglBindAPI(EGL_OPENGL_API);

    const EGLint cfgAttribs[] = {
        EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
        EGL_SURFACE_TYPE,    EGL_WINDOW_BIT,
        EGL_RED_SIZE,        8,
        EGL_GREEN_SIZE,      8,
        EGL_BLUE_SIZE,       8,
        EGL_ALPHA_SIZE,      8,
        EGL_DEPTH_SIZE,      24,
        EGL_STENCIL_SIZE,    8,
        EGL_NONE
    };
    EGLint numConfigs = 0;
    if (!eglChooseConfig(m_native->eglDisplay, cfgAttribs, &m_native->eglConfig, 1, &numConfigs)
        || numConfigs < 1)
        return false;

    const EGLint ctxAttribs[] = {
        EGL_CONTEXT_MAJOR_VERSION, 3,
        EGL_CONTEXT_MINOR_VERSION, 3,
        EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT,
        EGL_NONE
    };
    m_native->eglContext = eglCreateContext(m_native->eglDisplay, m_native->eglConfig,
                                            EGL_NO_CONTEXT, ctxAttribs);
    if (m_native->eglContext == EGL_NO_CONTEXT)
        return false;

    m_native->eglWindow = wl_egl_window_create(m_native->wlSurface, w, h);
    if (!m_native->eglWindow)
        return false;

    m_native->eglSurface = eglCreateWindowSurface(
        m_native->eglDisplay, m_native->eglConfig,
        static_cast<EGLNativeWindowType>(m_native->eglWindow), nullptr);
    if (m_native->eglSurface == EGL_NO_SURFACE)
        return false;

    eglSwapInterval(m_native->eglDisplay, 1);

    m_valid = true;
    return true;
}

void TzGLContext::resize(int w, int h)
{
    if (m_native->eglWindow && (w != m_native->width || h != m_native->height)) {
        m_native->width  = w;
        m_native->height = h;
        wl_egl_window_resize(m_native->eglWindow, w, h, 0, 0);
    }
}

void TzGLContext::makeCurrent()
{
    eglMakeCurrent(m_native->eglDisplay, m_native->eglSurface,
                   m_native->eglSurface, m_native->eglContext);
}

void TzGLContext::doneCurrent()
{
    eglMakeCurrent(m_native->eglDisplay, EGL_NO_SURFACE,
                   EGL_NO_SURFACE, EGL_NO_CONTEXT);
}

void TzGLContext::swapBuffers()
{
    eglSwapBuffers(m_native->eglDisplay, m_native->eglSurface);
}

void TzGLContext::destroy()
{
    if (m_native->eglDisplay == EGL_NO_DISPLAY)
        return;

    eglMakeCurrent(m_native->eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

    if (m_native->eglSurface != EGL_NO_SURFACE) {
        eglDestroySurface(m_native->eglDisplay, m_native->eglSurface);
        m_native->eglSurface = EGL_NO_SURFACE;
    }
    if (m_native->eglWindow) {
        wl_egl_window_destroy(m_native->eglWindow);
        m_native->eglWindow = nullptr;
    }
    if (m_native->eglContext != EGL_NO_CONTEXT) {
        eglDestroyContext(m_native->eglDisplay, m_native->eglContext);
        m_native->eglContext = EGL_NO_CONTEXT;
    }
    eglTerminate(m_native->eglDisplay);
    m_native->eglDisplay = EGL_NO_DISPLAY;
    m_valid = false;
}
