#include "tzwaylandglobals.hpp"
#include "tzwaylandwindow.hpp"

#include <loom/tzcoreapplication.hpp>
#include <loom/tzkeyevent.hpp>
#include <loom/tzmouseevent.hpp>

#include "protocol/xdg-decoration-client-protocol.h"

#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <sys/mman.h>
#include <unistd.h>

// ── xkb helpers ──────────────────────────────────────────────────────────────

static Key xkbSymToKey(xkb_keysym_t sym)
{
    switch (sym) {
    case XKB_KEY_Escape:
        return Key::Escape;
    case XKB_KEY_Return:
        return Key::Enter;
    case XKB_KEY_KP_Enter:
        return Key::Enter;
    case XKB_KEY_Tab:
        return Key::Tab;
    case XKB_KEY_BackSpace:
        return Key::Backspace;
    case XKB_KEY_Up:
        return Key::Up;
    case XKB_KEY_Down:
        return Key::Down;
    case XKB_KEY_Left:
        return Key::Left;
    case XKB_KEY_Right:
        return Key::Right;
    case XKB_KEY_Home:
        return Key::Home;
    case XKB_KEY_End:
        return Key::End;
    case XKB_KEY_Page_Up:
        return Key::PageUp;
    case XKB_KEY_Page_Down:
        return Key::PageDown;
    case XKB_KEY_F1:
        return Key::F1;
    case XKB_KEY_F2:
        return Key::F2;
    case XKB_KEY_F3:
        return Key::F3;
    case XKB_KEY_F4:
        return Key::F4;
    case XKB_KEY_F5:
        return Key::F5;
    case XKB_KEY_F6:
        return Key::F6;
    case XKB_KEY_F7:
        return Key::F7;
    case XKB_KEY_F8:
        return Key::F8;
    case XKB_KEY_F9:
        return Key::F9;
    case XKB_KEY_F10:
        return Key::F10;
    case XKB_KEY_F11:
        return Key::F11;
    case XKB_KEY_F12:
        return Key::F12;
    default:
        return Key::Unknown;
    }
}

static KeyModifiers currentMods(xkb_state *state)
{
    KeyModifiers mods;
    if (xkb_state_mod_name_is_active(state, XKB_MOD_NAME_SHIFT, XKB_STATE_MODS_EFFECTIVE) > 0)
        mods |= KeyModifier::Shift;
    if (xkb_state_mod_name_is_active(state, XKB_MOD_NAME_CTRL, XKB_STATE_MODS_EFFECTIVE) > 0)
        mods |= KeyModifier::Ctrl;
    if (xkb_state_mod_name_is_active(state, XKB_MOD_NAME_ALT, XKB_STATE_MODS_EFFECTIVE) > 0)
        mods |= KeyModifier::Alt;
    return mods;
}

// ── wl_keyboard listener ─────────────────────────────────────────────────────
// All callbacks receive TzWaylandGlobals* via data to avoid calling instance()
// during static initialization (which would cause recursive_init_error).

static void kbd_keymap(void *data, wl_keyboard *, uint32_t format, int fd, uint32_t size)
{
    auto *g = static_cast<TzWaylandGlobals *>(data);
    if (format != WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1) {
        close(fd);
        return;
    }
    void *map = mmap(nullptr, size, PROT_READ, MAP_SHARED, fd, 0);
    close(fd);
    if (map == MAP_FAILED)
        return;

    if (g->xkbKeymap)
        xkb_keymap_unref(g->xkbKeymap);
    if (g->xkbState)
        xkb_state_unref(g->xkbState);

    g->xkbKeymap = xkb_keymap_new_from_string(g->xkbCtx, static_cast<char *>(map),
                                              XKB_KEYMAP_FORMAT_TEXT_V1,
                                              XKB_KEYMAP_COMPILE_NO_FLAGS);
    munmap(map, size);

    g->xkbState = g->xkbKeymap ? xkb_state_new(g->xkbKeymap) : nullptr;
}

static void kbd_enter(void *data, wl_keyboard *, uint32_t, wl_surface *surface, wl_array *)
{
    auto *g = static_cast<TzWaylandGlobals *>(data);
    g->keyboardFocus = g->windowForSurface(surface);
}

static void kbd_leave(void *data, wl_keyboard *, uint32_t, wl_surface *)
{
    static_cast<TzWaylandGlobals *>(data)->keyboardFocus = nullptr;
}

static void kbd_key(void *data, wl_keyboard *, uint32_t, uint32_t, uint32_t key, uint32_t state)
{
    auto *g = static_cast<TzWaylandGlobals *>(data);
    if (!g->xkbState || !g->keyboardFocus)
        return;

    uint32_t keycode = key + 8; // evdev → XKB offset
    xkb_keysym_t sym = xkb_state_key_get_one_sym(g->xkbState, keycode);
    Key k = xkbSymToKey(sym);
    KeyModifiers mods = currentMods(g->xkbState);

    std::string utf8;
    if (k == Key::Unknown) {
        char buf[32];
        int n = xkb_state_key_get_utf8(g->xkbState, keycode, buf, static_cast<int>(sizeof(buf)));
        if (n > 0)
            utf8.assign(buf, static_cast<size_t>(n));
    }

    bool pressed = (state == WL_KEYBOARD_KEY_STATE_PRESSED);
    TzCoreApplication::postEvent(g->keyboardFocus,
                                 new TzKeyEvent(pressed ? TzEvent::KeyPress : TzEvent::KeyRelease,
                                                k, mods, std::move(utf8)));
}

static void kbd_modifiers(void *data, wl_keyboard *, uint32_t, uint32_t mods_dep, uint32_t mods_lat,
                          uint32_t mods_lock, uint32_t group)
{
    auto *g = static_cast<TzWaylandGlobals *>(data);
    if (g->xkbState)
        xkb_state_update_mask(g->xkbState, mods_dep, mods_lat, mods_lock, 0, 0, group);
}

static void kbd_repeat_info(void *, wl_keyboard *, int32_t, int32_t) {}

static const wl_keyboard_listener kKbdListener = {kbd_keymap, kbd_enter,     kbd_leave,
                                                  kbd_key,    kbd_modifiers, kbd_repeat_info};

// ── wl_pointer listener ───────────────────────────────────────────────────────

static void ptr_enter(void *data, wl_pointer *pointer, uint32_t serial, wl_surface *surface,
                      wl_fixed_t sx, wl_fixed_t sy)
{
    auto *g = static_cast<TzWaylandGlobals *>(data);
    g->pointerFocus = g->windowForSurface(surface);
    g->pointerX = wl_fixed_to_double(sx);
    g->pointerY = wl_fixed_to_double(sy);
    g->pointerSerial = serial;
    g->setCursor("left_ptr");
}

static void ptr_leave(void *data, wl_pointer *, uint32_t, wl_surface *)
{
    static_cast<TzWaylandGlobals *>(data)->pointerFocus = nullptr;
}

static void ptr_motion(void *data, wl_pointer *, uint32_t, wl_fixed_t sx, wl_fixed_t sy)
{
    auto *g = static_cast<TzWaylandGlobals *>(data);
    g->pointerX = wl_fixed_to_double(sx);
    g->pointerY = wl_fixed_to_double(sy);
    if (!g->pointerFocus)
        return;
    KeyModifiers mods = g->xkbState ? currentMods(g->xkbState) : KeyModifiers{};
    TzCoreApplication::postEvent(g->pointerFocus,
                                 new TzMouseEvent(TzEvent::MouseMove, MouseButton::None,
                                                  g->pointerX, g->pointerY, mods));
}

static void ptr_button(void *data, wl_pointer *, uint32_t, uint32_t, uint32_t button, uint32_t state)
{
    auto *g = static_cast<TzWaylandGlobals *>(data);
    if (!g->pointerFocus)
        return;

    MouseButton mb = MouseButton::None;
    if (button == 0x110)
        mb = MouseButton::Left;
    else if (button == 0x111)
        mb = MouseButton::Right;
    else if (button == 0x112)
        mb = MouseButton::Middle;

    bool pressed = (state == WL_POINTER_BUTTON_STATE_PRESSED);
    KeyModifiers mods = g->xkbState ? currentMods(g->xkbState) : KeyModifiers{};
    TzCoreApplication::postEvent(g->pointerFocus,
                                 new TzMouseEvent(pressed ? TzEvent::MouseButtonPress
                                                          : TzEvent::MouseButtonRelease,
                                                  mb, g->pointerX, g->pointerY, mods));
}

static void ptr_axis(void *data, wl_pointer *, uint32_t, uint32_t axis, wl_fixed_t value)
{
    auto *g = static_cast<TzWaylandGlobals *>(data);
    if (!g->pointerFocus)
        return;
    double delta = wl_fixed_to_double(value);
    double dx = 0.0, dy = 0.0;
    if (axis == WL_POINTER_AXIS_VERTICAL_SCROLL)
        dy = -delta / 15.0;
    else
        dx = -delta / 15.0;
    KeyModifiers mods = g->xkbState ? currentMods(g->xkbState) : KeyModifiers{};
    TzCoreApplication::postEvent(g->pointerFocus,
                                 new TzMouseEvent(TzEvent::MouseScroll, MouseButton::None,
                                                  g->pointerX, g->pointerY, mods, dx, dy));
}

// No-op stubs for wl_pointer events added in protocol versions 5+ that we
// don't need to handle but must provide to avoid NULL-function-pointer crashes.
static void ptr_frame(void *, wl_pointer *) {}
static void ptr_axis_source(void *, wl_pointer *, uint32_t) {}
static void ptr_axis_stop(void *, wl_pointer *, uint32_t, uint32_t) {}
static void ptr_axis_discrete(void *, wl_pointer *, uint32_t, int32_t) {}
static void ptr_axis_value120(void *, wl_pointer *, uint32_t, int32_t) {}
static void ptr_axis_rel_dir(void *, wl_pointer *, uint32_t, uint32_t) {}

static const wl_pointer_listener kPtrListener = {ptr_enter,         ptr_leave,
                                                 ptr_motion,        ptr_button,
                                                 ptr_axis,          ptr_frame,
                                                 ptr_axis_source,   ptr_axis_stop,
                                                 ptr_axis_discrete, ptr_axis_value120,
                                                 ptr_axis_rel_dir};

// ── wl_seat listener ─────────────────────────────────────────────────────────

static void seat_capabilities(void *data, wl_seat *seat, uint32_t caps)
{
    auto *g = static_cast<TzWaylandGlobals *>(data);

    bool hasKbd = (caps & WL_SEAT_CAPABILITY_KEYBOARD) != 0;
    bool hasPtr = (caps & WL_SEAT_CAPABILITY_POINTER) != 0;

    if (hasKbd && !g->keyboard) {
        g->keyboard = wl_seat_get_keyboard(seat);
        wl_keyboard_add_listener(g->keyboard, &kKbdListener, g);
    } else if (!hasKbd && g->keyboard) {
        wl_keyboard_destroy(g->keyboard);
        g->keyboard = nullptr;
    }

    if (hasPtr && !g->pointer) {
        g->pointer = wl_seat_get_pointer(seat);
        wl_pointer_add_listener(g->pointer, &kPtrListener, g);
    } else if (!hasPtr && g->pointer) {
        wl_pointer_destroy(g->pointer);
        g->pointer = nullptr;
    }
}

static void seat_name(void *, wl_seat *, const char *) {}

static const wl_seat_listener kSeatListener = {seat_capabilities, seat_name};

// ── xdg_wm_base listener ─────────────────────────────────────────────────────

static void xdg_base_ping(void *, xdg_wm_base *base, uint32_t serial)
{
    xdg_wm_base_pong(base, serial);
}

static const xdg_wm_base_listener kXdgWmBaseListener = {xdg_base_ping};

// ── wl_registry listener ─────────────────────────────────────────────────────

static void registry_global(void *data, wl_registry *registry, uint32_t name, const char *iface,
                            uint32_t version)
{
    // Use the pointer passed as user data — do NOT call instance() here.
    // instance() would try to re-enter the static-local initializer and throw
    // recursive_init_error on GCC's implementation.
    auto *g = static_cast<TzWaylandGlobals *>(data);

    if (strcmp(iface, wl_compositor_interface.name) == 0) {
        g->compositor = static_cast<wl_compositor *>(
            wl_registry_bind(registry, name, &wl_compositor_interface, version < 4u ? version : 4u));
    } else if (strcmp(iface, wl_shm_interface.name) == 0) {
        g->shm = static_cast<wl_shm *>(wl_registry_bind(registry, name, &wl_shm_interface, 1));
    } else if (strcmp(iface, xdg_wm_base_interface.name) == 0) {
        g->xdgWmBase = static_cast<xdg_wm_base *>(
            wl_registry_bind(registry, name, &xdg_wm_base_interface, version < 2u ? version : 2u));
        xdg_wm_base_add_listener(g->xdgWmBase, &kXdgWmBaseListener, nullptr);
    } else if (strcmp(iface, wl_seat_interface.name) == 0) {
        g->seat = static_cast<wl_seat *>(
            wl_registry_bind(registry, name, &wl_seat_interface, version < 5u ? version : 5u));
        wl_seat_add_listener(g->seat, &kSeatListener, g);
    } else if (strcmp(iface, zxdg_decoration_manager_v1_interface.name) == 0) {
        g->decorationManager = static_cast<zxdg_decoration_manager_v1 *>(
            wl_registry_bind(registry, name, &zxdg_decoration_manager_v1_interface, 1));
    }
}

static void registry_global_remove(void *, wl_registry *, uint32_t) {}

static const wl_registry_listener kRegistryListener = {registry_global, registry_global_remove};

// ── TzWaylandGlobals ─────────────────────────────────────────────────────────

TzWaylandGlobals &TzWaylandGlobals::instance()
{
    static TzWaylandGlobals inst;
    return inst;
}

TzWaylandGlobals::TzWaylandGlobals()
{
    display = wl_display_connect(nullptr);
    if (!display)
        throw std::runtime_error("wl_display_connect failed — is a Wayland compositor running?");

    registry = wl_display_get_registry(display);
    // Pass 'this' so registry_global can fill our members without calling instance().
    wl_registry_add_listener(registry, &kRegistryListener, this);
    wl_display_roundtrip(display); // bind globals
    wl_display_roundtrip(display); // allow seat capabilities to arrive

    if (!compositor)
        throw std::runtime_error("Wayland compositor not available");
    if (!shm)
        throw std::runtime_error("Wayland wl_shm not available");
    if (!xdgWmBase)
        throw std::runtime_error(
            "xdg_wm_base not available — compositor does not support XDG shell");

    xkbCtx = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    if (!xkbCtx)
        throw std::runtime_error("xkb_context_new failed");

    // Load cursor theme (size 24, or honour XCURSOR_SIZE env var).
    int cursorSize = 24;
    if (const char *env = std::getenv("XCURSOR_SIZE"))
        if (int s = std::atoi(env); s > 0)
            cursorSize = s;
    cursorTheme = wl_cursor_theme_load(nullptr, cursorSize, shm);
    if (cursorTheme && compositor)
        cursorSurface = wl_compositor_create_surface(compositor);
}

TzWaylandGlobals::~TzWaylandGlobals()
{
    if (xkbState)
        xkb_state_unref(xkbState);
    if (xkbKeymap)
        xkb_keymap_unref(xkbKeymap);
    if (xkbCtx)
        xkb_context_unref(xkbCtx);
    if (cursorSurface)
        wl_surface_destroy(cursorSurface);
    if (cursorTheme)
        wl_cursor_theme_destroy(cursorTheme);
    if (pointer)
        wl_pointer_destroy(pointer);
    if (keyboard)
        wl_keyboard_destroy(keyboard);
    if (seat)
        wl_seat_destroy(seat);
    if (decorationManager)
        zxdg_decoration_manager_v1_destroy(decorationManager);
    if (xdgWmBase)
        xdg_wm_base_destroy(xdgWmBase);
    if (shm)
        wl_shm_destroy(shm);
    if (compositor)
        wl_compositor_destroy(compositor);
    if (registry)
        wl_registry_destroy(registry);
    if (display)
        wl_display_disconnect(display);
}

void TzWaylandGlobals::setCursor(const char *name)
{
    if (!cursorTheme || !cursorSurface || !pointer)
        return;

    wl_cursor *cursor = wl_cursor_theme_get_cursor(cursorTheme, name);
    if (!cursor && name != nullptr)
        cursor = wl_cursor_theme_get_cursor(cursorTheme, "left_ptr");
    if (!cursor)
        return;

    cursorImage = cursor->images[0];
    wl_buffer *buf = wl_cursor_image_get_buffer(cursorImage);
    if (!buf)
        return;

    wl_pointer_set_cursor(pointer, pointerSerial, cursorSurface,
                          static_cast<int32_t>(cursorImage->hotspot_x),
                          static_cast<int32_t>(cursorImage->hotspot_y));
    wl_surface_attach(cursorSurface, buf, 0, 0);
    wl_surface_damage(cursorSurface, 0, 0, static_cast<int32_t>(cursorImage->width),
                      static_cast<int32_t>(cursorImage->height));
    wl_surface_commit(cursorSurface);
}

void TzWaylandGlobals::registerSurface(wl_surface *surface, TzWaylandWindow *window)
{
    if (m_surfaceCount < 16)
        m_surfaces[m_surfaceCount++] = {surface, window};
}

void TzWaylandGlobals::unregisterSurface(wl_surface *surface)
{
    for (int i = 0; i < m_surfaceCount; i++) {
        if (m_surfaces[i].surface == surface) {
            if (keyboardFocus == m_surfaces[i].window)
                keyboardFocus = nullptr;
            if (pointerFocus == m_surfaces[i].window)
                pointerFocus = nullptr;
            m_surfaces[i] = m_surfaces[--m_surfaceCount];
            return;
        }
    }
}

TzWaylandWindow *TzWaylandGlobals::windowForSurface(wl_surface *surface) const
{
    for (int i = 0; i < m_surfaceCount; i++)
        if (m_surfaces[i].surface == surface)
            return m_surfaces[i].window;
    return nullptr;
}

int TzWaylandGlobals::fd() const
{
    return wl_display_get_fd(display);
}

void TzWaylandGlobals::flush()
{
    wl_display_flush(display);
}

void TzWaylandGlobals::dispatchPending()
{
    wl_display_dispatch_pending(display);
}

bool TzWaylandGlobals::prepareRead()
{
    return wl_display_prepare_read(display) == 0;
}

void TzWaylandGlobals::readEvents()
{
    wl_display_read_events(display);
}

void TzWaylandGlobals::cancelRead()
{
    wl_display_cancel_read(display);
}

void TzWaylandGlobals::roundtrip()
{
    wl_display_roundtrip(display);
}
