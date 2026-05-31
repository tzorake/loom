#include "tzglcontext.hpp"

#include <GL/glx.h>
#include <X11/Xlib.h>

typedef GLXContext (*PFNGLXCREATECONTEXTATTRIBSARBPROC)(
    Display *, GLXFBConfig, GLXContext, Bool, const int *);

#define GLX_CONTEXT_MAJOR_VERSION_ARB    0x2091
#define GLX_CONTEXT_MINOR_VERSION_ARB    0x2092
#define GLX_CONTEXT_PROFILE_MASK_ARB     0x9126
#define GLX_CONTEXT_CORE_PROFILE_BIT_ARB 0x00000001

struct TzGLContext::NativeData
{
    Display    *display{nullptr};
    GLXWindow   glxWindow{0};
    GLXContext  glxContext{nullptr};
    Window      xWindow{0};
    bool        ownsDisplay{false};
};

TzGLContext::TzGLContext()
    : m_native(new NativeData)
{}

TzGLContext::~TzGLContext()
{
    destroy();
    delete m_native;
}

bool TzGLContext::create(const TzNativeWindowHandle &handle, int /*w*/, int /*h*/)
{
    m_native->display = static_cast<Display *>(handle.display);
    m_native->xWindow = static_cast<Window>(handle.window);

    if (!m_native->display)
        return false;

    int screen = handle.screen;

    // Choose a framebuffer config
    int attribs[] = {
        GLX_X_RENDERABLE, True,
        GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
        GLX_RENDER_TYPE,   GLX_RGBA_BIT,
        GLX_DOUBLEBUFFER,  True,
        GLX_RED_SIZE,    8,
        GLX_GREEN_SIZE,  8,
        GLX_BLUE_SIZE,   8,
        GLX_ALPHA_SIZE,  8,
        GLX_DEPTH_SIZE,  24,
        GLX_STENCIL_SIZE, 8,
        None
    };
    int count = 0;
    GLXFBConfig *configs = glXChooseFBConfig(m_native->display, screen, attribs, &count);
    if (!configs || count == 0)
        return false;
    GLXFBConfig cfg = configs[0];
    XFree(configs);

    auto glXCreateContextAttribsARB =
        (PFNGLXCREATECONTEXTATTRIBSARBPROC)
        glXGetProcAddressARB((const GLubyte *)"glXCreateContextAttribsARB");
    if (!glXCreateContextAttribsARB)
        return false;

    int ctxAttribs[] = {
        GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
        GLX_CONTEXT_MINOR_VERSION_ARB, 3,
        GLX_CONTEXT_PROFILE_MASK_ARB,  GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
        None
    };
    m_native->glxContext = glXCreateContextAttribsARB(
        m_native->display, cfg, nullptr, True, ctxAttribs);
    if (!m_native->glxContext)
        return false;

    m_native->glxWindow = glXCreateWindow(m_native->display, cfg, m_native->xWindow, nullptr);

    m_valid = true;
    return true;
}

void TzGLContext::resize(int /*w*/, int /*h*/) {}

void TzGLContext::makeCurrent()
{
    glXMakeContextCurrent(m_native->display,
                          m_native->glxWindow,
                          m_native->glxWindow,
                          m_native->glxContext);
}

void TzGLContext::doneCurrent()
{
    glXMakeContextCurrent(m_native->display, None, None, nullptr);
}

void TzGLContext::swapBuffers()
{
    glXSwapBuffers(m_native->display, m_native->glxWindow);
}

void TzGLContext::destroy()
{
    if (!m_native->display)
        return;
    if (m_native->glxContext) {
        glXMakeContextCurrent(m_native->display, None, None, nullptr);
        glXDestroyContext(m_native->display, m_native->glxContext);
        m_native->glxContext = nullptr;
    }
    if (m_native->glxWindow) {
        glXDestroyWindow(m_native->display, m_native->glxWindow);
        m_native->glxWindow = 0;
    }
    m_valid = false;
}
