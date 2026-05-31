#include "tzglcontext.hpp"

#import <Cocoa/Cocoa.h>
#import <OpenGL/OpenGL.h>
#import <OpenGL/gl3.h>

struct TzGLContext::NativeData
{
    NSOpenGLContext *glContext{nil};
    NSView          *view{nil};
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
    NSOpenGLPixelFormatAttribute attrs[] = {
        NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion4_1Core,
        NSOpenGLPFADoubleBuffer,
        NSOpenGLPFAColorSize,   24,
        NSOpenGLPFAAlphaSize,    8,
        NSOpenGLPFADepthSize,   24,
        NSOpenGLPFAStencilSize,  8,
        0
    };

    NSOpenGLPixelFormat *fmt = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
    if (!fmt) {
        // Fall back to GL 3.3 Core
        NSOpenGLPixelFormatAttribute attrs33[] = {
            NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion3_2Core,
            NSOpenGLPFADoubleBuffer,
            NSOpenGLPFAColorSize, 24,
            NSOpenGLPFAAlphaSize,  8,
            NSOpenGLPFADepthSize, 24,
            0
        };
        fmt = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs33];
        if (!fmt)
            return false;
    }

    m_native->glContext = [[NSOpenGLContext alloc] initWithFormat:fmt shareContext:nil];
    [fmt release];
    if (!m_native->glContext)
        return false;

    m_native->view = static_cast<NSView *>(handle.nsView);
    [m_native->glContext setView:m_native->view];

    m_valid = true;
    return true;
}

void TzGLContext::resize(int /*w*/, int /*h*/)
{
    if (m_native->glContext)
        [m_native->glContext update];
}

void TzGLContext::makeCurrent()
{
    if (m_native->glContext)
        [m_native->glContext makeCurrentContext];
}

void TzGLContext::doneCurrent()
{
    [NSOpenGLContext clearCurrentContext];
}

void TzGLContext::swapBuffers()
{
    if (m_native->glContext)
        [m_native->glContext flushBuffer];
}

void TzGLContext::destroy()
{
    if (m_native->glContext) {
        [NSOpenGLContext clearCurrentContext];
        [m_native->glContext release];
        m_native->glContext = nil;
    }
    m_native->view = nil;
    m_valid = false;
}
