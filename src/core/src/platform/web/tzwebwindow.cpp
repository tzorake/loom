#include "tzwebwindow.hpp"
#include "tzwebimports.hpp"

#include <loom/tzcloseevent.hpp>
#include <loom/tzcoreapplication.hpp>
#include <loom/tzkeyevent.hpp>
#include <loom/tzmouseevent.hpp>
#include <loom/tzresizeevent.hpp>

// ── Module-level singleton ────────────────────────────────────────────────

TzWebWindow *TzWebWindow::s_instance = nullptr;

TzWebWindow *TzWebWindow::instance()
{
    return s_instance;
}

// ── Key-code translation (JS keyCode → Key enum) ──────────────────────────
//
// JavaScript KeyboardEvent.keyCode values for non-printable keys.

static Key translateKeyCode(int code)
{
    switch (code) {
    case   8: return Key::Backspace;
    case   9: return Key::Tab;
    case  13: return Key::Enter;
    case  27: return Key::Escape;
    case  33: return Key::PageUp;
    case  34: return Key::PageDown;
    case  35: return Key::End;
    case  36: return Key::Home;
    case  37: return Key::Left;
    case  38: return Key::Up;
    case  39: return Key::Right;
    case  40: return Key::Down;
    case 112: return Key::F1;
    case 113: return Key::F2;
    case 114: return Key::F3;
    case 115: return Key::F4;
    case 116: return Key::F5;
    case 117: return Key::F6;
    case 118: return Key::F7;
    case 119: return Key::F8;
    case 120: return Key::F9;
    case 121: return Key::F10;
    case 122: return Key::F11;
    case 123: return Key::F12;
    default:  return Key::Unknown;
    }
}

// ── Exported event-delivery functions called from JavaScript ──────────────
//
// JavaScript calls these functions to push DOM events into the C++ event queue.
// They are exported under well-known names so the JS loader can find them
// on instance.exports without name mangling.

WASM_EXPORT_AS("loom_mouse_move")
void loom_mouse_move(int x, int y)
{
    TzWebWindow *win = TzWebWindow::instance();
    if (!win) return;
    TzCoreApplication::postEvent(
        win,
        new TzMouseEvent(TzEvent::MouseMove, MouseButton::None,
                         static_cast<double>(x), static_cast<double>(y),
                         KeyModifier::None));
}

WASM_EXPORT_AS("loom_mouse_button")
void loom_mouse_button(int jsButton, int pressed, int x, int y)
{
    TzWebWindow *win = TzWebWindow::instance();
    if (!win) return;

    MouseButton button = MouseButton::None;
    switch (jsButton) {
    case 0: button = MouseButton::Left;   break;
    case 1: button = MouseButton::Middle; break;
    case 2: button = MouseButton::Right;  break;
    }

    int type = pressed ? TzEvent::MouseButtonPress : TzEvent::MouseButtonRelease;
    TzCoreApplication::postEvent(
        win,
        new TzMouseEvent(type, button,
                         static_cast<double>(x), static_cast<double>(y),
                         KeyModifier::None));
}

WASM_EXPORT_AS("loom_mouse_scroll")
void loom_mouse_scroll(int x, int y, double dx, double dy)
{
    TzWebWindow *win = TzWebWindow::instance();
    if (!win) return;
    TzCoreApplication::postEvent(
        win,
        new TzMouseEvent(TzEvent::MouseScroll, MouseButton::None,
                         static_cast<double>(x), static_cast<double>(y),
                         KeyModifier::None, dx, dy));
}

// loom_key_event — called by JS with:
//   jsKeyCode : KeyboardEvent.keyCode
//   pressed   : 1 for keydown, 0 for keyup
//   shift/ctrl/alt : modifier booleans
//   utf8_0..3 : up to 4 bytes of the UTF-8 encoded character (0 = unused)
//               Only populated for printable keys (key == Key::Unknown).
WASM_EXPORT_AS("loom_key_event")
void loom_key_event(int jsKeyCode, int pressed,
                    int shift, int ctrl, int alt,
                    int utf8_0, int utf8_1, int utf8_2, int utf8_3)
{
    TzWebWindow *win = TzWebWindow::instance();
    if (!win) return;

    Key key = translateKeyCode(jsKeyCode);

    KeyModifiers mods;
    if (shift) mods |= KeyModifier::Shift;
    if (ctrl)  mods |= KeyModifier::Ctrl;
    if (alt)   mods |= KeyModifier::Alt;

    std::string utf8;
    if (key == Key::Unknown && utf8_0 > 0) {
        utf8 += static_cast<char>(utf8_0);
        if (utf8_1 > 0) utf8 += static_cast<char>(utf8_1);
        if (utf8_2 > 0) utf8 += static_cast<char>(utf8_2);
        if (utf8_3 > 0) utf8 += static_cast<char>(utf8_3);
    }

    int type = pressed ? TzEvent::KeyPress : TzEvent::KeyRelease;
    TzCoreApplication::postEvent(
        win,
        new TzKeyEvent(type, key, mods, std::move(utf8)));
}

WASM_EXPORT_AS("loom_resize")
void loom_resize(int width, int height)
{
    TzWebWindow *win = TzWebWindow::instance();
    if (win)
        win->onResize(width, height);
}

WASM_EXPORT_AS("loom_close")
void loom_close()
{
    TzWebWindow *win = TzWebWindow::instance();
    if (win)
        TzCoreApplication::postEvent(win, new TzCloseEvent);
}

// ── TzWebWindow ───────────────────────────────────────────────────────────

TzWebWindow::TzWebWindow(int width, int height)
{
    // Query the actual canvas size from JS; fall back to the requested size.
    int cw = js_get_canvas_width();
    int ch = js_get_canvas_height();
    m_width  = (cw > 0) ? cw : width;
    m_height = (ch > 0) ? ch : height;

    s_instance = this;
}

TzWebWindow::~TzWebWindow()
{
    if (s_instance == this)
        s_instance = nullptr;
}

void TzWebWindow::setTitle(const std::string &title)
{
    js_set_title(title.c_str(), static_cast<int>(title.size()));
}

void TzWebWindow::show()
{
    js_show_canvas(1);

    // Post an initial resize so widgets layout and paint before the first RAF.
    TzResizeEvent re(m_width, m_height);
    TzCoreApplication::sendEvent(this, &re);
}

void TzWebWindow::hide()
{
    js_show_canvas(0);
}

void TzWebWindow::onResize(int width, int height)
{
    m_width  = width;
    m_height = height;
    TzCoreApplication::postEvent(this, new TzResizeEvent(width, height));
}

void TzWebWindow::render(const std::vector<uint32_t> &pixels, int width, int height)
{
    // The framework stores pixels as 0xAARRGGBB.
    // On little-endian WASM memory that is byte layout: B G R A.
    // Canvas ImageData expects byte layout:             R G B A.
    // Convert by swapping the R and B bytes of each pixel.
    m_rgba.resize(pixels.size());
    for (std::size_t i = 0; i < pixels.size(); ++i) {
        uint32_t p = pixels[i];
        uint8_t  a = static_cast<uint8_t>((p >> 24) & 0xFF);
        uint8_t  r = static_cast<uint8_t>((p >> 16) & 0xFF);
        uint8_t  g = static_cast<uint8_t>((p >>  8) & 0xFF);
        uint8_t  b = static_cast<uint8_t>( p        & 0xFF);
        // Produce little-endian uint32 with byte layout R G B A.
        m_rgba[i] = (static_cast<uint32_t>(a) << 24)
                  | (static_cast<uint32_t>(b) << 16)
                  | (static_cast<uint32_t>(g) <<  8)
                  |  static_cast<uint32_t>(r);
    }
    js_draw_pixels(m_rgba.data(), 0, 0, width, height);
}
