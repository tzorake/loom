#include "tzx11globals.hpp"
#include "tzx11window.hpp"

#include <loom/tzcoreapplication.hpp>
#include <loom/tzkeyevent.hpp>
#include <loom/tzmouseevent.hpp>
#include <loom/tzresizeevent.hpp>
#include <loom/tzcloseevent.hpp>

#include <stdexcept>

static Key x11SymToKey(KeySym sym)
{
    switch (sym) {
        case XK_Escape:    return Key::Escape;
        case XK_Return:    return Key::Enter;
        case XK_KP_Enter:  return Key::Enter;
        case XK_Tab:       return Key::Tab;
        case XK_BackSpace: return Key::Backspace;
        case XK_Up:        return Key::Up;
        case XK_Down:      return Key::Down;
        case XK_Left:      return Key::Left;
        case XK_Right:     return Key::Right;
        case XK_Home:      return Key::Home;
        case XK_End:       return Key::End;
        case XK_Page_Up:   return Key::PageUp;
        case XK_Page_Down: return Key::PageDown;
        case XK_F1:        return Key::F1;
        case XK_F2:        return Key::F2;
        case XK_F3:        return Key::F3;
        case XK_F4:        return Key::F4;
        case XK_F5:        return Key::F5;
        case XK_F6:        return Key::F6;
        case XK_F7:        return Key::F7;
        case XK_F8:        return Key::F8;
        case XK_F9:        return Key::F9;
        case XK_F10:       return Key::F10;
        case XK_F11:       return Key::F11;
        case XK_F12:       return Key::F12;
        default:           return Key::Unknown;
    }
}

static KeyModifiers x11Modifiers(unsigned int state)
{
    KeyModifiers mods;
    if (state & ShiftMask)   mods |= KeyModifier::Shift;
    if (state & ControlMask) mods |= KeyModifier::Ctrl;
    if (state & Mod1Mask)    mods |= KeyModifier::Alt;
    return mods;
}

TzX11Globals &TzX11Globals::instance()
{
    static TzX11Globals inst;
    return inst;
}

TzX11Globals::TzX11Globals()
{
    display = XOpenDisplay(nullptr);
    if (!display)
        throw std::runtime_error("XOpenDisplay failed — is DISPLAY set?");

    screen  = DefaultScreen(display);
    visual  = DefaultVisual(display, screen);
    depth   = DefaultDepth(display, screen);
    rootWin = RootWindow(display, screen);

    wmDeleteWindow = XInternAtom(display, "WM_DELETE_WINDOW", 0);
    netWmName      = XInternAtom(display, "_NET_WM_NAME",     0);
    utf8String     = XInternAtom(display, "UTF8_STRING",      0);
}

TzX11Globals::~TzX11Globals()
{
    if (display) {
        XCloseDisplay(display);
        display = nullptr;
    }
}

int TzX11Globals::fd() const
{
    return ConnectionNumber(display);
}

void TzX11Globals::flush()
{
    XFlush(display);
}

void TzX11Globals::registerWindow(Window xwin, TzX11Window *win)
{
    if (m_count < 32)
        m_windows[m_count++] = { xwin, win };
}

void TzX11Globals::unregisterWindow(Window xwin)
{
    for (int i = 0; i < m_count; i++) {
        if (m_windows[i].xwin == xwin) {
            m_windows[i] = m_windows[--m_count];
            return;
        }
    }
}

TzX11Window *TzX11Globals::windowForXWindow(Window xwin) const
{
    for (int i = 0; i < m_count; i++)
        if (m_windows[i].xwin == xwin)
            return m_windows[i].win;
    return nullptr;
}

void TzX11Globals::dispatchPendingEvents()
{
    while (XPending(display)) {
        XEvent ev;
        XNextEvent(display, &ev);

        TzX11Window *win = windowForXWindow(ev.xany.window);
        if (!win)
            continue;

        switch (ev.type) {
        case X11ev::kConfigureNotify:
            win->onConfigure(ev.xconfigure.width, ev.xconfigure.height);
            break;

        case X11ev::kExpose:
            if (ev.xexpose.count == 0)  // last in a sequence — repaint once
                win->onExpose();
            break;

        case X11ev::kKeyPress:
        case X11ev::kKeyRelease: {
            char buf[32];
            KeySym sym;
            int n = XLookupString(&ev.xkey, buf, sizeof(buf) - 1, &sym, nullptr);
            Key k = x11SymToKey(sym);
            KeyModifiers mods = x11Modifiers(ev.xkey.state);
            std::string utf8;
            if (k == Key::Unknown && n > 0)
                utf8.assign(buf, static_cast<size_t>(n));
            bool pressed = (ev.type == X11ev::kKeyPress);
            TzCoreApplication::postEvent(win,
                new TzKeyEvent(pressed ? TzEvent::KeyPress : TzEvent::KeyRelease,
                               k, mods, std::move(utf8)));
            break;
        }

        case X11ev::kButtonPress:
        case X11ev::kButtonRelease: {
            bool pressed = (ev.type == X11ev::kButtonPress);
            KeyModifiers mods = x11Modifiers(ev.xbutton.state);
            double x = static_cast<double>(ev.xbutton.x);
            double y = static_cast<double>(ev.xbutton.y);
            unsigned btn = ev.xbutton.button;

            if (btn == 4 || btn == 5) {
                if (pressed) {
                    double dy = (btn == 4) ? 1.0 : -1.0;
                    TzCoreApplication::postEvent(win,
                        new TzMouseEvent(TzEvent::MouseScroll, MouseButton::None,
                                         x, y, mods, 0.0, dy));
                }
            } else {
                MouseButton mb = MouseButton::None;
                if      (btn == 1) mb = MouseButton::Left;
                else if (btn == 2) mb = MouseButton::Middle;
                else if (btn == 3) mb = MouseButton::Right;
                TzCoreApplication::postEvent(win,
                    new TzMouseEvent(pressed ? TzEvent::MouseButtonPress
                                             : TzEvent::MouseButtonRelease,
                                     mb, x, y, mods));
            }
            break;
        }

        case X11ev::kMotionNotify: {
            KeyModifiers mods = x11Modifiers(ev.xmotion.state);
            TzCoreApplication::postEvent(win,
                new TzMouseEvent(TzEvent::MouseMove, MouseButton::None,
                                 static_cast<double>(ev.xmotion.x),
                                 static_cast<double>(ev.xmotion.y),
                                 mods));
            break;
        }

        case X11ev::kClientMessage:
            if (static_cast<Atom>(ev.xclient.data.l[0]) == wmDeleteWindow) {
                TzCloseEvent closeEv;
                TzCoreApplication::sendEvent(win, &closeEv);
                if (closeEv.isAccepted())
                    win->hide();
            }
            break;

        default:
            break;
        }
    }
}
