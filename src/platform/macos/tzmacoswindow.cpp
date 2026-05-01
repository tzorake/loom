#include <event-loop/tzmacoswindow.hpp>
#include <event-loop/tzflags.hpp>

#include "tzmacoswindow_p.hpp"
#include "tzobjcutils.hpp"

#include <CoreGraphics/CoreGraphics.h>

#include <mutex>
#include <stdexcept>
// #include <type_traits>

// template <auto EnumValue>
// constexpr auto as_underlying_v = static_cast<std::underlying_type_t<decltype(EnumValue)>>(EnumValue);

using NSUInteger = unsigned long;

// /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/System/Library/Frameworks/CoreGraphics.framework/Headers
// /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/System/Library/Frameworks/AppKit.framework/Versions/C/Headers/NSWindow.h

// NSWindowStyleMask
constexpr NSUInteger NSWindowStyleMaskBorderless = 0;
constexpr NSUInteger NSWindowStyleMaskTitled = 1 << 0;
constexpr NSUInteger NSWindowStyleMaskClosable = 1 << 1;
constexpr NSUInteger NSWindowStyleMaskMiniaturizable = 1 << 2;
constexpr NSUInteger NSWindowStyleMaskResizable	= 1 << 3;
constexpr NSUInteger NSWindowStyleMaskTexturedBackground = 1 << 8;
constexpr NSUInteger NSWindowStyleMaskUnifiedTitleAndToolbar = 1 << 12;
constexpr NSUInteger NSWindowStyleMaskFullScreen = 1 << 14;
constexpr NSUInteger NSWindowStyleMaskFullSizeContentView = 1 << 15;
constexpr NSUInteger NSWindowStyleMaskUtilityWindow = 1 << 4;
constexpr NSUInteger NSWindowStyleMaskDocModalWindow = 1 << 6;
constexpr NSUInteger NSWindowStyleMaskNonactivatingPanel = 1 << 7;
constexpr NSUInteger NSWindowStyleMaskHUDWindow = 1 << 13;

// NSBackingStoreType
constexpr NSUInteger NSBackingStoreRetained = 0;
constexpr NSUInteger NSBackingStoreNonretained = 1;
constexpr NSUInteger NSBackingStoreBuffered = 2;

static constexpr const char *kPrivateIvar = "tzPrivate";

static void *getPrivate(ObjcObject self)
{
    void *ptr = nullptr;
    object_getInstanceVariable(self, kPrivateIvar, &ptr);
    return ptr;
}

static BOOL windowShouldClose_impl(ObjcObject self, objc_selector*, ObjcObject)
{
    TzMacosWindowPrivate *d = static_cast<TzMacosWindowPrivate*>(getPrivate(self));
    return (d && d->onWindowShouldClose()) ? YES : NO;
}

static void drawRect_impl(ObjcObject self, objc_selector*, CGRect rect)
{
    TzMacosWindowPrivate *d = static_cast<TzMacosWindowPrivate*>(getPrivate(self));
    if (d) d->onDrawRect(self, rect);
}

static void windowDidResize_impl(ObjcObject self, objc_selector*, ObjcObject /*notification*/)
{
    TzMacosWindowPrivate *d = static_cast<TzMacosWindowPrivate*>(getPrivate(self));
    if (d) d->onWindowDidResize();
}

// Content view — keyboard
static void keyDown_impl(ObjcObject self, objc_selector*, ObjcObject nsEvent)
{
    TzMacosWindowPrivate *d = static_cast<TzMacosWindowPrivate*>(getPrivate(self));
    if (d) d->onKeyEvent(nsEvent, true);
}

static void keyUp_impl(ObjcObject self, objc_selector*, ObjcObject nsEvent)
{
    TzMacosWindowPrivate *d = static_cast<TzMacosWindowPrivate*>(getPrivate(self));
    if (d) d->onKeyEvent(nsEvent, false);
}

static void flagsChanged_impl(ObjcObject self, objc_selector*, ObjcObject nsEvent)
{
    // Treat modifier-only events as press (state is in modifierFlags, not keyCode)
    TzMacosWindowPrivate *d = static_cast<TzMacosWindowPrivate*>(getPrivate(self));
    if (d) d->onKeyEvent(nsEvent, true);
}

// Content view — mouse
static void mouseDown_impl(ObjcObject self, objc_selector*, ObjcObject nsEvent)
{
    TzMacosWindowPrivate *d = static_cast<TzMacosWindowPrivate*>(getPrivate(self));
    if (d) d->onMouseEvent(nsEvent, TzEvent::MouseButtonPress, MouseButton::Left);
}

static void mouseUp_impl(ObjcObject self, objc_selector*, ObjcObject nsEvent)
{
    TzMacosWindowPrivate *d = static_cast<TzMacosWindowPrivate*>(getPrivate(self));
    if (d) d->onMouseEvent(nsEvent, TzEvent::MouseButtonRelease, MouseButton::Left);
}

static void rightMouseDown_impl(ObjcObject self, objc_selector*, ObjcObject nsEvent)
{
    TzMacosWindowPrivate *d = static_cast<TzMacosWindowPrivate*>(getPrivate(self));
    if (d) d->onMouseEvent(nsEvent, TzEvent::MouseButtonPress, MouseButton::Right);
}

static void rightMouseUp_impl(ObjcObject self, objc_selector*, ObjcObject nsEvent)
{
    TzMacosWindowPrivate *d = static_cast<TzMacosWindowPrivate*>(getPrivate(self));
    if (d) d->onMouseEvent(nsEvent, TzEvent::MouseButtonRelease, MouseButton::Right);
}

static void otherMouseDown_impl(ObjcObject self, objc_selector*, ObjcObject nsEvent)
{
    TzMacosWindowPrivate *d = static_cast<TzMacosWindowPrivate*>(getPrivate(self));
    if (d) d->onMouseEvent(nsEvent, TzEvent::MouseButtonPress, MouseButton::Middle);
}

static void otherMouseUp_impl(ObjcObject self, objc_selector*, ObjcObject nsEvent)
{
    TzMacosWindowPrivate *d = static_cast<TzMacosWindowPrivate*>(getPrivate(self));
    if (d) d->onMouseEvent(nsEvent, TzEvent::MouseButtonRelease, MouseButton::Middle);
}

static void mouseMoved_impl(ObjcObject self, objc_selector*, ObjcObject nsEvent)
{
    TzMacosWindowPrivate *d = static_cast<TzMacosWindowPrivate*>(getPrivate(self));
    if (d) d->onMouseEvent(nsEvent, TzEvent::MouseMove, MouseButton::None);
}

static void mouseDragged_impl(ObjcObject self, objc_selector*, ObjcObject nsEvent)
{
    TzMacosWindowPrivate *d = static_cast<TzMacosWindowPrivate*>(getPrivate(self));
    if (d) d->onMouseEvent(nsEvent, TzEvent::MouseMove, MouseButton::None);
}

static void scrollWheel_impl(ObjcObject self, objc_selector*, ObjcObject nsEvent)
{
    TzMacosWindowPrivate *d = static_cast<TzMacosWindowPrivate*>(getPrivate(self));
    if (d) d->onMouseEvent(nsEvent, TzEvent::MouseScroll, MouseButton::None);
}

static BOOL acceptsFirstResponder_impl(ObjcObject /*self*/, objc_selector*)
{
    return YES;
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
        // Keyboard
        class_addMethod(gContentViewClass, sel_registerName("keyDown:"),
            reinterpret_cast<ObjcMethodImpl>(keyDown_impl), "v@:@");
        class_addMethod(gContentViewClass, sel_registerName("keyUp:"),
            reinterpret_cast<ObjcMethodImpl>(keyUp_impl), "v@:@");
        class_addMethod(gContentViewClass, sel_registerName("flagsChanged:"),
            reinterpret_cast<ObjcMethodImpl>(flagsChanged_impl), "v@:@");
        class_addMethod(gContentViewClass, sel_registerName("acceptsFirstResponder"),
            reinterpret_cast<ObjcMethodImpl>(acceptsFirstResponder_impl), "c@:");
        // Mouse
        class_addMethod(gContentViewClass, sel_registerName("mouseDown:"),
            reinterpret_cast<ObjcMethodImpl>(mouseDown_impl), "v@:@");
        class_addMethod(gContentViewClass, sel_registerName("mouseUp:"),
            reinterpret_cast<ObjcMethodImpl>(mouseUp_impl), "v@:@");
        class_addMethod(gContentViewClass, sel_registerName("rightMouseDown:"),
            reinterpret_cast<ObjcMethodImpl>(rightMouseDown_impl), "v@:@");
        class_addMethod(gContentViewClass, sel_registerName("rightMouseUp:"),
            reinterpret_cast<ObjcMethodImpl>(rightMouseUp_impl), "v@:@");
        class_addMethod(gContentViewClass, sel_registerName("otherMouseDown:"),
            reinterpret_cast<ObjcMethodImpl>(otherMouseDown_impl), "v@:@");
        class_addMethod(gContentViewClass, sel_registerName("otherMouseUp:"),
            reinterpret_cast<ObjcMethodImpl>(otherMouseUp_impl), "v@:@");
        class_addMethod(gContentViewClass, sel_registerName("mouseMoved:"),
            reinterpret_cast<ObjcMethodImpl>(mouseMoved_impl), "v@:@");
        class_addMethod(gContentViewClass, sel_registerName("mouseDragged:"),
            reinterpret_cast<ObjcMethodImpl>(mouseDragged_impl), "v@:@");
        class_addMethod(gContentViewClass, sel_registerName("scrollWheel:"),
            reinterpret_cast<ObjcMethodImpl>(scrollWheel_impl), "v@:@");
        objc_registerClassPair(gContentViewClass);
    });
}

TzMacosWindowPrivate::TzMacosWindowPrivate(int width, int height, TzAbstractWindow *owner)
    : owner(owner)
    , windowWidth(width)
    , windowHeight(height)
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
        NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable,
        NSBackingStoreBuffered,
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

    // Enable mouse-moved events (off by default in Cocoa)
    sendMessage<void>(window, "setAcceptsMouseMovedEvents:", (int)YES);
    sendMessage<void>(window, "makeFirstResponder:", contentView);
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
    TzCloseEvent event;
    TzCoreApplication::sendEvent(owner, &event);
    return event.isAccepted();
}

void TzMacosWindowPrivate::onWindowDidResize()
{
    CGRect bounds = sendMessage<CGRect>(contentView, "bounds");
    windowWidth  = (int)bounds.size.width;
    windowHeight = (int)bounds.size.height;
    TzCoreApplication::postEvent(owner, new TzResizeEvent(windowWidth, windowHeight));
}

static constexpr NSUInteger kNSShiftKeyMask   = 1 << 17;
static constexpr NSUInteger kNSControlKeyMask = 1 << 18;
static constexpr NSUInteger kNSAlternateKeyMask = 1 << 19;
static constexpr NSUInteger kNSCommandKeyMask = 1 << 20;

static KeyModifiers translateModifiers(NSUInteger flags)
{
    KeyModifiers mods;
    if (flags & kNSShiftKeyMask) mods |= KeyModifier::Shift;
    if (flags & kNSControlKeyMask) mods |= KeyModifier::Ctrl;
    if (flags & kNSAlternateKeyMask) mods |= KeyModifier::Alt;
    return mods;
}

// NSEvent keyCode values for special keys
static Key translateKeyCode(unsigned short keyCode)
{
    switch (keyCode) {
        case 0x35: return Key::Escape;
        case 0x24: return Key::Enter;
        case 0x4C: return Key::Enter; // numpad enter
        case 0x30: return Key::Tab;
        case 0x33: return Key::Backspace;
        case 0x7E: return Key::Up;
        case 0x7D: return Key::Down;
        case 0x7B: return Key::Left;
        case 0x7C: return Key::Right;
        case 0x73: return Key::Home;
        case 0x77: return Key::End;
        case 0x74: return Key::PageUp;
        case 0x79: return Key::PageDown;
        case 0x7A: return Key::F1;
        case 0x78: return Key::F2;
        case 0x63: return Key::F3;
        case 0x76: return Key::F4;
        case 0x60: return Key::F5;
        case 0x61: return Key::F6;
        case 0x62: return Key::F7;
        case 0x64: return Key::F8;
        case 0x65: return Key::F9;
        case 0x6D: return Key::F10;
        case 0x67: return Key::F11;
        case 0x6F: return Key::F12;
        default:   return Key::Unknown;
    }
}

void TzMacosWindowPrivate::onKeyEvent(ObjcObject nsEvent, bool pressed)
{
    NSUInteger flags = sendMessage<NSUInteger>(nsEvent, "modifierFlags");
    unsigned short code = sendMessage<unsigned short>(nsEvent, "keyCode");

    Key key = translateKeyCode(code);
    KeyModifiers mods = translateModifiers(flags);

    std::string utf8;
    if (key == Key::Unknown) {
        ObjcObject chars = sendMessage<ObjcObject>(nsEvent, "characters");
        if (chars) {
            const char *str = sendMessage<const char *>(chars, "UTF8String");
            if (str) utf8 = str;
        }
    }

    TzCoreApplication::postEvent(owner,
        new TzKeyEvent(pressed ? TzEvent::KeyPress : TzEvent::KeyRelease, key, mods, std::move(utf8)));
}

void TzMacosWindowPrivate::onMouseEvent(ObjcObject nsEvent, TzEvent::Type type, MouseButton button)
{
    // Convert from window coords to view (flipped: origin top-left)
    CGPoint windowPos = sendMessage<CGPoint>(nsEvent, "locationInWindow");
    CGPoint viewPos = sendMessage<CGPoint>(contentView, "convertPoint:fromView:", windowPos, nullptr);
    CGRect bounds = sendMessage<CGRect>(contentView, "bounds");
    NSUInteger flags = sendMessage<NSUInteger>(nsEvent, "modifierFlags");

    double scrollDx = 0.0, scrollDy = 0.0;
    if (type == TzEvent::MouseScroll) {
        scrollDx = sendMessage<double>(nsEvent, "scrollingDeltaX");
        scrollDy = sendMessage<double>(nsEvent, "scrollingDeltaY");
    }

    TzCoreApplication::postEvent(owner,
        new TzMouseEvent(type, button,
                         viewPos.x, bounds.size.height - viewPos.y, // flip to top-left origin
                         translateModifiers(flags),
                         scrollDx, scrollDy));
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
        static_cast<uint32_t>(kCGImageAlphaFirst) | kCGBitmapByteOrder32Big,
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

TzMacosWindow::TzMacosWindow(int width, int height)
    : d_ptr(new TzMacosWindowPrivate(width, height, this))
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

void TzMacosWindow::render(const std::vector<uint32_t>& pixels, int width, int height)
{
    d_ptr->render(pixels, width, height);
}
