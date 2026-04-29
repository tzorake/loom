#include <event-loop/tzmacoswindow.hpp>
#include <event-loop/tzflags.hpp>

#include "tzmacoswindow_p.hpp"
#include "tzobjcutils.hpp"

#include <CoreGraphics/CoreGraphics.h>

#include <mutex>
#include <stdexcept>

using NSUInteger = unsigned long;

enum class WindowStyle : NSUInteger {
    Titled = 1 << 0,
    Closable = 1 << 1,
    Miniaturizable = 1 << 2,
    Resizable = 1 << 3,
};
TZ_DECLARE_FLAGS(WindowStyles, WindowStyle)
TZ_DECLARE_OPERATORS_FOR_FLAGS(WindowStyles)

// ---------------------------------------------------------------------------
// ObjC class registration (once per process)
// ---------------------------------------------------------------------------

static constexpr const char *kPrivateIvar = "tzPrivate";

static void* getPrivate(ObjcObject self)
{
    void* ptr = nullptr;
    object_getInstanceVariable(self, kPrivateIvar, &ptr);
    return ptr;
}

static BOOL windowShouldClose_impl(ObjcObject self, objc_selector*, ObjcObject)
{
    auto* d = static_cast<TzMacosWindowPrivate*>(getPrivate(self));
    return (d && d->onWindowShouldClose()) ? YES : NO;
}

static void drawRect_impl(ObjcObject self, objc_selector*, CGRect rect)
{
    auto* d = static_cast<TzMacosWindowPrivate*>(getPrivate(self));
    if (d) d->onDrawRect(self, rect);
}

static void windowDidResize_impl(ObjcObject self, objc_selector*, ObjcObject /*notification*/)
{
    auto* d = static_cast<TzMacosWindowPrivate*>(getPrivate(self));
    if (d) d->onWindowDidResize();
}

static ObjcClass gDelegateClass   = nullptr;
static ObjcClass gContentViewClass = nullptr;
static std::once_flag gClassRegistration;

static void registerObjcClasses()
{
    std::call_once(gClassRegistration, []() {
        // Delegate class
        gDelegateClass = objc_allocateClassPair(getClass("NSObject"), "TzWindowDelegate", 0);
        class_addIvar(gDelegateClass, kPrivateIvar, sizeof(void*), __alignof(void*), "^v");
        class_addMethod(gDelegateClass, sel_registerName("windowShouldClose:"),
            reinterpret_cast<ObjcMethodImpl>(windowShouldClose_impl), "c@:@");
        class_addMethod(gDelegateClass, sel_registerName("windowDidResize:"),
            reinterpret_cast<ObjcMethodImpl>(windowDidResize_impl), "v@:@");
        objc_registerClassPair(gDelegateClass);

        // Content view class
        gContentViewClass = objc_allocateClassPair(getClass("NSView"), "TzContentView", 0);
        class_addIvar(gContentViewClass, kPrivateIvar, sizeof(void*), __alignof(void*), "^v");
        class_addMethod(gContentViewClass, sel_registerName("drawRect:"),
            reinterpret_cast<ObjcMethodImpl>(drawRect_impl),
            "v@:{CGRect={CGPoint=dd}{CGSize=dd}}");
        objc_registerClassPair(gContentViewClass);
    });
}

// ---------------------------------------------------------------------------
// TzMacosWindowPrivate
// ---------------------------------------------------------------------------

TzMacosWindowPrivate::TzMacosWindowPrivate(int width, int height)
    : windowWidth(width), windowHeight(height)
{
    registerObjcClasses();

    // Ensure NSApplication is initialised so windows can be created
    ObjcObject app = sendClassMessage<ObjcObject>(getClass("NSApplication"), "sharedApplication");
    sendMessage<void>(app, "setActivationPolicy:", (NSUInteger)0 /* NSApplicationActivationPolicyRegular */);

    // Center window on screen
    ObjcObject screen = sendClassMessage<ObjcObject>(getClass("NSScreen"), "mainScreen");
    CGRect screenFrame = sendMessage<CGRect>(screen, "frame");
    CGRect windowRect{
        (screenFrame.size.width  - width)  / 2.0,
        (screenFrame.size.height - height) / 2.0,
        (double)width,
        (double)height
    };

    // Create NSWindow
    ObjcObject win = sendClassMessage<ObjcObject>(getClass("NSWindow"), "alloc");
    window = sendMessage<ObjcObject>(win,
        "initWithContentRect:styleMask:backing:defer:",
        windowRect,
        WindowStyle::Titled | WindowStyle::Closable | WindowStyle::Miniaturizable | WindowStyle::Resizable,
        (NSUInteger)2 /* NSBackingStoreBuffered */,
        (int)NO
    );

    // Create delegate and wire private pointer
    ObjcObject del = sendClassMessage<ObjcObject>(gDelegateClass, "alloc");
    delegate = sendMessage<ObjcObject>(del, "init");
    object_setInstanceVariable(delegate, kPrivateIvar, this);
    sendMessage<void>(window, "setDelegate:", delegate);

    // Create content view and wire private pointer
    ObjcObject existingBounds = sendMessage<ObjcObject>(
        sendMessage<ObjcObject>(window, "contentView"), "bounds");
    CGRect contentBounds = sendMessage<CGRect>(
        sendMessage<ObjcObject>(window, "contentView"), "bounds");

    ObjcObject cv = sendClassMessage<ObjcObject>(gContentViewClass, "alloc");
    contentView = sendMessage<ObjcObject>(cv, "initWithFrame:", contentBounds);
    object_setInstanceVariable(contentView, kPrivateIvar, this);
    sendMessage<void>(window, "setContentView:", contentView);
    (void)existingBounds;
}

TzMacosWindowPrivate::~TzMacosWindowPrivate()
{
    // Clear private pointers so stale callbacks are safe
    object_setInstanceVariable(delegate, kPrivateIvar, nullptr);
    object_setInstanceVariable(contentView, kPrivateIvar, nullptr);

    sendMessage<void>(window, "close");
}

void TzMacosWindowPrivate::setTitle(const std::string& title)
{
    ObjcObject nsTitle = sendClassMessage<ObjcObject>(
        getClass("NSString"), "stringWithUTF8String:", title.data());
    sendMessage<void>(window, "setTitle:", nsTitle);
}

void TzMacosWindowPrivate::show()
{
    ObjcObject app = sendClassMessage<ObjcObject>(getClass("NSApplication"), "sharedApplication");
    sendMessage<void>(window, "makeKeyAndOrderFront:", nullptr);
    sendMessage<void>(app, "activateIgnoringOtherApps:", (int)YES);
}

void TzMacosWindowPrivate::hide()
{
    sendMessage<void>(window, "orderOut:", nullptr);
}

void TzMacosWindowPrivate::render(const std::vector<uint32_t>& newPixels, int width, int height)
{
    {
        std::lock_guard lock(pixelMutex);
        pixels    = newPixels;
        pixelWidth  = width;
        pixelHeight = height;
    }
    // Request redraw on the main thread
    sendMessage<void>(contentView, "setNeedsDisplay:", (int)YES);
}

bool TzMacosWindowPrivate::onWindowShouldClose()
{
    if (closeCallback)
        closeCallback();
    return true;
}

void TzMacosWindowPrivate::onWindowDidResize()
{
    CGRect bounds = sendMessage<CGRect>(contentView, "bounds");
    windowWidth  = (int)bounds.size.width;
    windowHeight = (int)bounds.size.height;
    if (resizeCallback)
        resizeCallback(windowWidth, windowHeight);
}

void TzMacosWindowPrivate::onDrawRect(ObjcObject self, CGRect /*rect*/)
{
    std::lock_guard lock(pixelMutex);

    if (pixels.empty())
        return;

    CGRect bounds = sendMessage<CGRect>(self, "bounds");

    ObjcObject nsCtx = sendClassMessage<ObjcObject>(getClass("NSGraphicsContext"), "currentContext");
    CGContextRef ctx = reinterpret_cast<CGContextRef>(
        sendMessage<ObjcObject>(nsCtx, "CGContext"));

    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();

    CGContextSaveGState(ctx);
    CGContextTranslateCTM(ctx, 0, bounds.size.height);
    CGContextScaleCTM(ctx, 1.0, -1.0);

    CGDataProviderRef provider = CGDataProviderCreateWithData(
        nullptr, pixels.data(), pixelWidth * pixelHeight * 4, nullptr);

    CGImageRef image = CGImageCreate(
        pixelWidth, pixelHeight,
        8, 32, pixelWidth * 4,
        colorSpace,
        kCGImageAlphaFirst | kCGBitmapByteOrder32Big,
        provider, nullptr, NO, kCGRenderingIntentDefault
    );

    if (image) {
        CGContextDrawImage(ctx, CGRectMake(0, 0, bounds.size.width, bounds.size.height), image);
        CGImageRelease(image);
    }

    CGDataProviderRelease(provider);
    CGColorSpaceRelease(colorSpace);
    CGContextRestoreGState(ctx);
}

// ---------------------------------------------------------------------------
// TzMacosWindow
// ---------------------------------------------------------------------------

TzMacosWindow::TzMacosWindow(int width, int height)
    : d_ptr(new TzMacosWindowPrivate(width, height))
{
}

TzMacosWindow::~TzMacosWindow() = default;

void TzMacosWindow::setTitle(const std::string& title)
{
    d_ptr->setTitle(title);
}

void TzMacosWindow::show()
{
    d_ptr->show();
}

void TzMacosWindow::hide()
{
    d_ptr->hide();
}

void TzMacosWindow::setCloseCallback(CloseCallback callback)
{
    d_ptr->closeCallback = std::move(callback);
}

void TzMacosWindow::setResizeCallback(ResizeCallback callback)
{
    d_ptr->resizeCallback = std::move(callback);
}

void TzMacosWindow::render(const std::vector<uint32_t>& pixels, int width, int height)
{
    d_ptr->render(pixels, width, height);
}
