#include "tzglcontext.hpp"
#include <loom/tzlogging.hpp>

#include <CoreFoundation/CoreFoundation.h>
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl3.h>

#include <objc/message.h>
#include <objc/objc.h>
#include <objc/runtime.h>

using ObjcObject = objc_object *;
using ObjcClass  = objc_class  *;

template<typename R = void, typename... Args>
static inline R objcSend(ObjcObject obj, const char *sel, Args... args)
{
    return reinterpret_cast<R (*)(ObjcObject, objc_selector *, Args...)>(
        objc_msgSend)(obj, sel_registerName(sel), args...);
}

template<typename R = void, typename... Args>
static inline R objcSendClass(ObjcClass cls, const char *sel, Args... args)
{
    return reinterpret_cast<R (*)(ObjcClass, objc_selector *, Args...)>(
        objc_msgSend)(cls, sel_registerName(sel), args...);
}

static inline ObjcClass getClass(const char *name)
{
    return objc_getClass(name);
}

// ── NSOpenGLPixelFormatAttribute constants (from AppKit headers) ──────────────

using NSUInteger = unsigned long;

// NSOpenGLPixelFormatAttribute is uint32_t — keep these as uint32_t so the
// attribute array passed to initWithAttributes: has the right element width.
static constexpr uint32_t kNSOpenGLPFAOpenGLProfile    = 99;
static constexpr uint32_t kNSOpenGLPFADoubleBuffer      = 5;
static constexpr uint32_t kNSOpenGLPFAColorSize         = 8;
static constexpr uint32_t kNSOpenGLPFAAlphaSize         = 11;
static constexpr uint32_t kNSOpenGLPFADepthSize         = 12;
static constexpr uint32_t kNSOpenGLPFAStencilSize       = 13;
static constexpr uint32_t kNSOpenGLProfileVersion4_1Core = 0x4100;
static constexpr uint32_t kNSOpenGLProfileVersion3_2Core = 0x3200;

// ── NativeData ────────────────────────────────────────────────────────────────

struct TzGLContext::NativeData
{
    ObjcObject glContext{nullptr};  // NSOpenGLContext *
    ObjcObject view{nullptr};       // NSView *
};

// ── TzGLContext ───────────────────────────────────────────────────────────────

TzGLContext::TzGLContext()
    : m_native(new NativeData)
{}

TzGLContext::~TzGLContext()
{
    destroy();
    delete m_native;
}

static ObjcObject makePixelFormat(uint32_t profile)
{
    // NSOpenGLPixelFormatAttribute is uint32_t (unsigned int), NOT NSUInteger
    // (unsigned long).  Using NSUInteger here would make each element 8 bytes
    // wide on 64-bit, so the 0-terminator would appear inside the second 32-bit
    // half of the first element — initWithAttributes: would see garbage/empty.
    uint32_t attrs[] = {
        kNSOpenGLPFAOpenGLProfile, profile,
        kNSOpenGLPFADoubleBuffer,
        kNSOpenGLPFAColorSize,    24,
        kNSOpenGLPFAAlphaSize,     8,
        kNSOpenGLPFADepthSize,    24,
        kNSOpenGLPFAStencilSize,   8,
        0
    };

    ObjcObject alloc = objcSendClass<ObjcObject>(
        getClass("NSOpenGLPixelFormat"), "alloc");
    return objcSend<ObjcObject>(alloc, "initWithAttributes:",
                                reinterpret_cast<const uint32_t *>(attrs));
}

bool TzGLContext::create(const TzNativeWindowHandle &handle, int /*w*/, int /*h*/)
{
    m_native->view = static_cast<ObjcObject>(handle.nsView);

    // Try GL 4.1 Core first, fall back to 3.2 Core (which gives us GL 3.3 on
    // macOS because Apple maps 3.2 Core → 4.1 on capable hardware).
    ObjcObject fmt = makePixelFormat(kNSOpenGLProfileVersion4_1Core);
    tzInfo("pixelFormat 4.1 = {}", (void *)fmt);
    if (!fmt) {
        fmt = makePixelFormat(kNSOpenGLProfileVersion3_2Core);
        tzInfo("pixelFormat 3.2 = {}", (void *)fmt);
    }
    if (!fmt) {
        tzError("no usable pixel format");
        return false;
    }

    ObjcObject alloc = objcSendClass<ObjcObject>(
        getClass("NSOpenGLContext"), "alloc");
    m_native->glContext = objcSend<ObjcObject>(
        alloc, "initWithFormat:shareContext:", fmt, nullptr);

    // Release the pixel format (NSOpenGLContext retains it internally).
    objcSend(fmt, "release");

    if (!m_native->glContext) {
        tzError("NSOpenGLContext alloc/init failed");
        return false;
    }

    // On macOS 14+ views are layer-backed by default, which is incompatible
    // with NSOpenGLContext.  Opt out explicitly before attaching.
    objcSend(m_native->view, "setWantsLayer:", (int)NO);

    // Attach to the NSView so swapBuffers can operate on its drawable.
    objcSend(m_native->glContext, "setView:", m_native->view);

    // Verify the drawable was set — if setView: fails the view() accessor returns nil.
    ObjcObject attachedView = objcSend<ObjcObject>(m_native->glContext, "view");
    tzInfo("NSOpenGLContext created, view={} (expected {})",
            (void*)attachedView, (void*)m_native->view);

    m_valid = true;
    return true;
}

void TzGLContext::resize(int /*w*/, int /*h*/)
{
    if (m_native->glContext)
        objcSend(m_native->glContext, "update");
}

void TzGLContext::makeCurrent()
{
    if (m_native->glContext)
        objcSend(m_native->glContext, "makeCurrentContext");
}

void TzGLContext::doneCurrent()
{
    // +clearCurrentContext is a class method on NSOpenGLContext.
    objcSendClass(getClass("NSOpenGLContext"), "clearCurrentContext");
}

void TzGLContext::swapBuffers()
{
    if (m_native->glContext)
        objcSend(m_native->glContext, "flushBuffer");
}

void TzGLContext::destroy()
{
    if (m_native->glContext) {
        objcSendClass(getClass("NSOpenGLContext"), "clearCurrentContext");
        objcSend(m_native->glContext, "release");
        m_native->glContext = nullptr;
    }
    m_native->view = nullptr;
    m_valid = false;
}
