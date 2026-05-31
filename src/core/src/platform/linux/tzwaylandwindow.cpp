#include "tzwaylandwindow.hpp"
#include "tzwaylandwindow_p.hpp"

#include <cstring>
#include <errno.h>
#include <stdexcept>

// ── SHM buffer helpers ────────────────────────────────────────────────────────

static void buffer_release(void *data, wl_buffer *)
{
    static_cast<ShmBuffer *>(data)->busy = false;
}

static const wl_buffer_listener kBufferListener = {buffer_release};

// ── xdg_surface / xdg_toplevel listeners ─────────────────────────────────────

static void xdg_surface_configure_cb(void *data, xdg_surface *xdg_surf, uint32_t serial)
{
    auto *d = static_cast<TzWaylandWindowPrivate *>(data);
    xdg_surface_ack_configure(xdg_surf, serial);
    d->configured = true;

    if (d->pendingWidth > 0 && d->pendingHeight > 0) {
        d->windowWidth = d->pendingWidth;
        d->windowHeight = d->pendingHeight;
        d->pendingWidth = 0;
        d->pendingHeight = 0;
    }

    // Send a resize event so the scene re-layouts and the paint timer renders
    // at the current (possibly new) dimensions.  If the scene is not yet wired
    // up (window still being constructed) this is a no-op.
    TzResizeEvent re(d->windowWidth, d->windowHeight);
    TzCoreApplication::sendEvent(d->owner, &re);

    // Commit to advance the compositor state machine.  If render() already
    // committed a buffer in the sendEvent chain above this is a harmless
    // redundant commit (no new damage or attach pending).
    wl_surface_commit(d->surface);
    TzWaylandGlobals::instance().flush();
}

static const xdg_surface_listener kXdgSurfaceListener = {xdg_surface_configure_cb};

static void xdg_toplevel_configure_cb(void *data, xdg_toplevel *, int32_t width, int32_t height,
                                      wl_array *)
{
    auto *d = static_cast<TzWaylandWindowPrivate *>(data);
    if (width > 0 && height > 0) {
        d->pendingWidth = width;
        d->pendingHeight = height;
    }
}

static void xdg_toplevel_close_cb(void *data, xdg_toplevel *)
{
    auto *d = static_cast<TzWaylandWindowPrivate *>(data);
    TzCloseEvent event;
    TzCoreApplication::sendEvent(d->owner, &event);
    if (event.isAccepted())
        d->owner->hide();
}

static const xdg_toplevel_listener kXdgToplevelListener = {xdg_toplevel_configure_cb,
                                                           xdg_toplevel_close_cb};

// ── TzWaylandWindowPrivate ────────────────────────────────────────────────────

TzWaylandWindowPrivate::TzWaylandWindowPrivate(int width, int height, TzWaylandWindow *owner)
    : owner(owner)
    , windowWidth(width)
    , windowHeight(height)
{
    auto &g = TzWaylandGlobals::instance();

    surface = wl_compositor_create_surface(g.compositor);
    if (!surface)
        throw std::runtime_error("wl_compositor_create_surface failed");

    xdgSurface = xdg_wm_base_get_xdg_surface(g.xdgWmBase, surface);
    if (!xdgSurface)
        throw std::runtime_error("xdg_wm_base_get_xdg_surface failed");

    xdg_surface_add_listener(xdgSurface, &kXdgSurfaceListener, this);

    xdgToplevel = xdg_surface_get_toplevel(xdgSurface);
    if (!xdgToplevel)
        throw std::runtime_error("xdg_surface_get_toplevel failed");

    xdg_toplevel_add_listener(xdgToplevel, &kXdgToplevelListener, this);

    if (g.decorationManager) {
        decoration = zxdg_decoration_manager_v1_get_toplevel_decoration(g.decorationManager,
                                                                        xdgToplevel);
        if (decoration)
            zxdg_toplevel_decoration_v1_set_mode(decoration,
                                                 ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE);
    }

    g.registerSurface(surface, owner);
    wl_surface_commit(surface);
    g.roundtrip(); // wait for initial configure
}

TzWaylandWindowPrivate::~TzWaylandWindowPrivate()
{
    auto &g = TzWaylandGlobals::instance();
    g.unregisterSurface(surface);

    buffers[0].destroyPool();
    buffers[1].destroyPool();

    if (decoration) {
        zxdg_toplevel_decoration_v1_destroy(decoration);
        decoration = nullptr;
    }
    if (xdgToplevel) {
        xdg_toplevel_destroy(xdgToplevel);
        xdgToplevel = nullptr;
    }
    if (xdgSurface) {
        xdg_surface_destroy(xdgSurface);
        xdgSurface = nullptr;
    }
    if (surface) {
        wl_surface_destroy(surface);
        surface = nullptr;
    }
}

void TzWaylandWindowPrivate::setTitle(const std::string &title)
{
    xdg_toplevel_set_title(xdgToplevel, title.c_str());
}

void TzWaylandWindowPrivate::show()
{
    visible = true;
    wl_surface_commit(surface);
    TzWaylandGlobals::instance().flush();
}

void TzWaylandWindowPrivate::hide()
{
    visible = false;
    wl_surface_attach(surface, nullptr, 0, 0);
    wl_surface_commit(surface);
    TzWaylandGlobals::instance().flush();
}

void TzWaylandWindowPrivate::render(const std::vector<uint32_t> &pixels, int width, int height)
{
    // Don't render before the initial configure or after hide().
    if (!configured || !visible)
        return;

    auto &g = TzWaylandGlobals::instance();

    // Pick a buffer slot not currently held by the compositor.
    int slot = currentBuffer;
    if (buffers[slot].busy) {
        slot ^= 1;
        if (buffers[slot].busy)
            return; // both slots held by compositor — skip frame, try next call
    }

    ShmBuffer &buf = buffers[slot];
    const size_t needed = static_cast<size_t>(width) * static_cast<size_t>(height) * 4;

    // Grow the shared pool if needed (cheap mremap, no fd recreation).
    if (!buf.ensureCapacity(g.shm, needed))
        return;

    // Recreate the wl_buffer object only when dimensions actually change.
    if (buf.width != width || buf.height != height) {
        buf.destroyBuffer();
        buf.wlBuf = wl_shm_pool_create_buffer(buf.pool, 0, width, height, width * 4,
                                              WL_SHM_FORMAT_ARGB8888);
        wl_buffer_add_listener(buf.wlBuf, &kBufferListener, &buf);
        buf.width = width;
        buf.height = height;
        buf.byteSize = needed;
    }

    // Copy pixels into the shared memory region (top-down, no flip needed).
    memcpy(buf.data, pixels.data(), needed);

    buf.busy = true;
    currentBuffer = slot ^ 1;

    wl_surface_attach(surface, buf.wlBuf, 0, 0);
    wl_surface_damage_buffer(surface, 0, 0, width, height);
    wl_surface_commit(surface);
    g.flush();
}

// ── TzWaylandWindow public API ────────────────────────────────────────────────

TzWaylandWindow::TzWaylandWindow(int width, int height)
    : d_ptr(new TzWaylandWindowPrivate(width, height, this))
{}

TzWaylandWindow::~TzWaylandWindow() = default;

void TzWaylandWindow::setTitle(const std::string &title)
{
    d_ptr->setTitle(title);
}

void TzWaylandWindow::show()
{
    d_ptr->show();
    TzResizeEvent re(d_ptr->windowWidth, d_ptr->windowHeight);
    TzCoreApplication::sendEvent(this, &re);
}

void TzWaylandWindow::hide()
{
    d_ptr->hide();
}

void TzWaylandWindow::render(const std::vector<uint32_t> &pixels, int width, int height)
{
    d_ptr->render(pixels, width, height);
}

TzNativeWindowHandle TzWaylandWindow::nativeWindowHandle() const
{
    TzNativeWindowHandle h;
    h.wlDisplay = static_cast<void *>(TzWaylandGlobals::instance().display);
    h.wlSurface = static_cast<void *>(d_ptr->surface);
    return h;
}
