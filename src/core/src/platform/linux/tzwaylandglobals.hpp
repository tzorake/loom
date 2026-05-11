#ifndef TZWAYLANDGLOBALS_HPP
#define TZWAYLANDGLOBALS_HPP

#include <wayland-client.h>
#include <wayland-cursor.h>
#include <xkbcommon/xkbcommon.h>
#include "protocol/xdg-shell-client-protocol.h"
#include "protocol/xdg-decoration-client-protocol.h"

class TzWaylandWindow;

class TzWaylandGlobals
{
public:
    static TzWaylandGlobals &instance();

    TzWaylandGlobals();
    ~TzWaylandGlobals();

    wl_display    *display    { nullptr };
    wl_registry   *registry   { nullptr };
    wl_compositor *compositor { nullptr };
    wl_shm        *shm        { nullptr };
    xdg_wm_base   *xdgWmBase  { nullptr };
    wl_seat       *seat              { nullptr };
    wl_keyboard   *keyboard          { nullptr };
    wl_pointer    *pointer           { nullptr };
    zxdg_decoration_manager_v1 *decorationManager { nullptr };

    // Cursor
    wl_cursor_theme  *cursorTheme   { nullptr };
    wl_surface       *cursorSurface { nullptr };
    wl_cursor_image  *cursorImage   { nullptr };
    uint32_t          pointerSerial { 0 };

    void setCursor(const char *name);

    xkb_context   *xkbCtx    { nullptr };
    xkb_keymap    *xkbKeymap  { nullptr };
    xkb_state     *xkbState   { nullptr };

    TzWaylandWindow *keyboardFocus { nullptr };
    TzWaylandWindow *pointerFocus  { nullptr };
    double           pointerX      { 0.0 };
    double           pointerY      { 0.0 };

    void registerSurface(wl_surface *surface, TzWaylandWindow *window);
    void unregisterSurface(wl_surface *surface);
    TzWaylandWindow *windowForSurface(wl_surface *surface) const;

    int  fd()            const;
    void flush();
    void dispatchPending();
    bool prepareRead();
    void readEvents();
    void cancelRead();
    void roundtrip();

private:
    struct SurfaceEntry { wl_surface *surface; TzWaylandWindow *window; };
    SurfaceEntry m_surfaces[16];
    int          m_surfaceCount { 0 };
};

#endif // TZWAYLANDGLOBALS_HPP
