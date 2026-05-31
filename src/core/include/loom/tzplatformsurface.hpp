#ifndef TZPLATFORMSURFACE_HPP
#define TZPLATFORMSURFACE_HPP

#include <cstdint>
#include <functional>
#include <vector>

class TzSurface;
class TzKeyEvent;
class TzMouseEvent;

// ── TzNativeWindowHandle ──────────────────────────────────────────────────────
//
// Opaque platform-specific data needed by the RHI to attach a GPU context to
// an existing native window.  Every TzPlatformSurface exposes one via
// nativeWindowHandle().  Members are void* to avoid pulling platform headers
// into this public header; callers in the RHI cast them to the appropriate
// platform type.

struct TzNativeWindowHandle
{
#if defined(LOOM_PLATFORM_WEB)
    // The CSS selector for the <canvas> element used for WebGL.
    // The canvas is always "#loom-canvas" in Loom's index.html.
    const char *canvasSelector{"#loom-canvas"};
#elif defined(__APPLE__)
    void *nsView{nullptr};    // NSView* — the Cocoa content view
    void *nsWindow{nullptr};  // NSWindow*
#elif defined(_WIN32)
    void *hwnd{nullptr};      // HWND
    void *hinstance{nullptr}; // HINSTANCE (may be null; use GetModuleHandle(nullptr) if needed)
#elif defined(LOOM_BACKEND_WAYLAND)
    void *wlDisplay{nullptr}; // wl_display*
    void *wlSurface{nullptr}; // wl_surface*
#else // X11 fallback
    void *display{nullptr};        // Display*
    unsigned long window{0};       // Window (XID)
    int           screen{0};
#endif
};

class TzPlatformSurface
{
public:
    using CloseCallback = std::function<void()>;
    using ResizeCallback = std::function<void(int, int)>;
    using KeyCallback = std::function<void(TzKeyEvent *)>;
    using MouseCallback = std::function<void(TzMouseEvent *)>;

    virtual ~TzPlatformSurface();

    // Back-pointer to the public TzSurface that owns this platform surface.
    virtual TzSurface *surface() const = 0;
    virtual void setSurface(TzSurface *surface) = 0;

    // Input / lifecycle routing
    virtual void setCloseCallback(CloseCallback cb) = 0;
    virtual void setResizeCallback(ResizeCallback cb) = 0;
    virtual void setKeyCallback(KeyCallback cb) = 0;
    virtual void setMouseCallback(MouseCallback cb) = 0;

    // Raster rendering
    virtual void render(const std::vector<uint32_t> &pixels, int width, int height) = 0;

    // Native window handle — used by the RHI to attach a GL/WebGL context.
    virtual TzNativeWindowHandle nativeWindowHandle() const = 0;
};

#endif // TZPLATFORMSURFACE_HPP
