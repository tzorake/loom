#include "tzx11window.hpp"
#include "tzx11window_p.hpp"

#include <stdexcept>

// ── TzX11WindowPrivate ────────────────────────────────────────────────────────

TzX11WindowPrivate::TzX11WindowPrivate(int width, int height, TzX11Window *owner)
    : owner(owner), windowWidth(width), windowHeight(height)
{
    auto &g = TzX11Globals::instance();
    display = g.display;

    XSetWindowAttributes attrs{};
    attrs.background_pixel = BlackPixel(display, g.screen);
    attrs.event_mask = (ExposureMask     |
                        KeyPressMask     | KeyReleaseMask   |
                        ButtonPressMask  | ButtonReleaseMask |
                        PointerMotionMask| StructureNotifyMask);

    window = XCreateWindow(
        display, g.rootWin,
        0, 0,
        static_cast<unsigned>(width), static_cast<unsigned>(height),
        0,
        g.depth, InputOutput, g.visual,
        CWBackPixel | CWEventMask, &attrs);

    if (!window)
        throw std::runtime_error("XCreateWindow failed");

    // Register WM_DELETE_WINDOW so clicking the close button sends a ClientMessage
    // instead of the server destroying the connection.
    XSetWMProtocols(display, window, &g.wmDeleteWindow, 1);

    gc = XCreateGC(display, window, 0, nullptr);

    g.registerWindow(window, owner);
    XFlush(display);
}

TzX11WindowPrivate::~TzX11WindowPrivate()
{
    TzX11Globals::instance().unregisterWindow(window);
    if (gc)     { XFreeGC(display, gc);     gc     = nullptr; }
    if (window) { XDestroyWindow(display, window); window = 0; }
    XFlush(display);
}

void TzX11WindowPrivate::setTitle(const std::string &title)
{
    auto &g = TzX11Globals::instance();
    XStoreName(display, window, title.c_str());
    XChangeProperty(display, window,
                    g.netWmName, g.utf8String, 8, PropModeReplace,
                    reinterpret_cast<const unsigned char *>(title.c_str()),
                    static_cast<int>(title.size()));
    XFlush(display);
}

void TzX11WindowPrivate::show()
{
    visible = true;
    XMapWindow(display, window);
    XFlush(display);
}

void TzX11WindowPrivate::hide()
{
    visible = false;
    XUnmapWindow(display, window);
    XFlush(display);
}

void TzX11WindowPrivate::render(const std::vector<uint32_t> &pixels, int width, int height)
{
    if (!visible) return;

    auto &g = TzX11Globals::instance();

    const size_t needed = static_cast<size_t>(width) * static_cast<size_t>(height) * 4;
    if (pixelBuf.size() < needed)
        pixelBuf.resize(needed);
    std::memcpy(pixelBuf.data(), pixels.data(), needed);
    lastRenderWidth  = width;
    lastRenderHeight = height;

    blitCache();
}

void TzX11WindowPrivate::blitCache()
{
    if (pixelBuf.empty() || lastRenderWidth <= 0 || lastRenderHeight <= 0)
        return;

    auto &g = TzX11Globals::instance();

    // XCreateImage wraps our buffer without copying; XPutImage sends it to the
    // X server synchronously, so the buffer remains valid throughout the call.
    XImage *img = XCreateImage(
        g.display, g.visual,
        static_cast<unsigned>(g.depth), ZPixmap, 0,
        pixelBuf.data(),
        static_cast<unsigned>(lastRenderWidth),
        static_cast<unsigned>(lastRenderHeight),
        32, lastRenderWidth * 4);
    if (!img) return;

    img->byte_order = LSBFirst;

    XPutImage(g.display, window, gc, img,
              0, 0, 0, 0,
              static_cast<unsigned>(lastRenderWidth),
              static_cast<unsigned>(lastRenderHeight));

    // Prevent XDestroyImage from freeing our buffer.
    img->data = nullptr;
    XDestroyImage(img);

    g.flush();
}

// ── TzX11Window public API ────────────────────────────────────────────────────

TzX11Window::TzX11Window(int width, int height)
    : d_ptr(new TzX11WindowPrivate(width, height, this))
{
}

TzX11Window::~TzX11Window() = default;

void TzX11Window::setTitle(const std::string &title)
{
    d_ptr->setTitle(title);
}

void TzX11Window::show()
{
    d_ptr->show();
    // Tell the scene about the initial window dimensions so it can layout and paint.
    TzResizeEvent re(d_ptr->windowWidth, d_ptr->windowHeight);
    TzCoreApplication::sendEvent(this, &re);
}

void TzX11Window::hide()
{
    d_ptr->hide();
}

void TzX11Window::render(const std::vector<uint32_t> &pixels, int width, int height)
{
    d_ptr->render(pixels, width, height);
}

void TzX11Window::onConfigure(int width, int height)
{
    if (width == d_ptr->windowWidth && height == d_ptr->windowHeight)
        return;
    d_ptr->windowWidth  = width;
    d_ptr->windowHeight = height;
    TzResizeEvent re(width, height);
    TzCoreApplication::sendEvent(this, &re);
}

void TzX11Window::onExpose()
{
    // Re-blit the cached frame when the window is uncovered.  If no frame has
    // been rendered yet the paint timer will draw the first one shortly.
    d_ptr->blitCache();
}
