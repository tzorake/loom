#include "tzglcontext.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// WGL extension entry points — loaded lazily on first use.
typedef HGLRC (WINAPI *PFNWGLCREATECONTEXTATTRIBSARBPROC)(HDC, HGLRC, const int *);
typedef BOOL  (WINAPI *PFNWGLSWAPINTERVALEXTPROC)(int);

static PFNWGLCREATECONTEXTATTRIBSARBPROC s_wglCreateContextAttribsARB = nullptr;
static PFNWGLSWAPINTERVALEXTPROC         s_wglSwapIntervalEXT         = nullptr;

// WGL_ARB_create_context attribute tokens
#define WGL_CONTEXT_MAJOR_VERSION_ARB     0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB     0x2092
#define WGL_CONTEXT_FLAGS_ARB             0x2094
#define WGL_CONTEXT_PROFILE_MASK_ARB      0x9126
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB  0x00000001

struct TzGLContext::NativeData
{
    HWND  hwnd{nullptr};
    HDC   hdc{nullptr};
    HGLRC hglrc{nullptr};
};

// Bootstrap: create a dummy GL 1.1 context so we can query the extension.
static bool loadWglExtensions()
{
    if (s_wglCreateContextAttribsARB)
        return true;

    WNDCLASSA wc{};
    wc.lpfnWndProc   = DefWindowProcA;
    wc.hInstance     = GetModuleHandleA(nullptr);
    wc.lpszClassName = "TzGLContextDummy";
    RegisterClassA(&wc);

    HWND dummyWnd = CreateWindowA("TzGLContextDummy", "", WS_OVERLAPPED,
                                  0, 0, 1, 1, nullptr, nullptr, wc.hInstance, nullptr);
    HDC  dummyDC  = GetDC(dummyWnd);

    PIXELFORMATDESCRIPTOR pfd{};
    pfd.nSize        = sizeof(pfd);
    pfd.nVersion     = 1;
    pfd.dwFlags      = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType   = PFD_TYPE_RGBA;
    pfd.cColorBits   = 32;
    pfd.cDepthBits   = 24;

    int fmt = ChoosePixelFormat(dummyDC, &pfd);
    SetPixelFormat(dummyDC, fmt, &pfd);
    HGLRC dummyCtx = wglCreateContext(dummyDC);
    wglMakeCurrent(dummyDC, dummyCtx);

    s_wglCreateContextAttribsARB =
        (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
    s_wglSwapIntervalEXT =
        (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");

    wglMakeCurrent(dummyDC, nullptr);
    wglDeleteContext(dummyCtx);
    ReleaseDC(dummyWnd, dummyDC);
    DestroyWindow(dummyWnd);
    return s_wglCreateContextAttribsARB != nullptr;
}

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
    if (!loadWglExtensions())
        return false;

    m_native->hwnd = static_cast<HWND>(handle.hwnd);
    m_native->hdc  = GetDC(m_native->hwnd);
    if (!m_native->hdc)
        return false;

    PIXELFORMATDESCRIPTOR pfd{};
    pfd.nSize      = sizeof(pfd);
    pfd.nVersion   = 1;
    pfd.dwFlags    = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 24;
    pfd.cStencilBits = 8;

    int fmt = ChoosePixelFormat(m_native->hdc, &pfd);
    if (!fmt || !SetPixelFormat(m_native->hdc, fmt, &pfd))
        return false;

    const int attribs[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
        WGL_CONTEXT_MINOR_VERSION_ARB, 3,
        WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        0
    };
    m_native->hglrc = s_wglCreateContextAttribsARB(m_native->hdc, nullptr, attribs);
    if (!m_native->hglrc)
        return false;

    if (s_wglSwapIntervalEXT) {
        wglMakeCurrent(m_native->hdc, m_native->hglrc);
        s_wglSwapIntervalEXT(1); // vsync on
        wglMakeCurrent(m_native->hdc, nullptr);
    }

    m_valid = true;
    return true;
}

void TzGLContext::resize(int /*w*/, int /*h*/)
{
    // Nothing needed for WGL — the DC stays valid across resizes.
}

void TzGLContext::makeCurrent()
{
    wglMakeCurrent(m_native->hdc, m_native->hglrc);
}

void TzGLContext::doneCurrent()
{
    wglMakeCurrent(m_native->hdc, nullptr);
}

void TzGLContext::swapBuffers()
{
    SwapBuffers(m_native->hdc);
}

void TzGLContext::destroy()
{
    if (m_native->hglrc) {
        wglMakeCurrent(m_native->hdc, nullptr);
        wglDeleteContext(m_native->hglrc);
        m_native->hglrc = nullptr;
    }
    if (m_native->hdc && m_native->hwnd) {
        ReleaseDC(m_native->hwnd, m_native->hdc);
        m_native->hdc = nullptr;
    }
    m_valid = false;
}
