#ifndef TZX11GLOBALS_HPP
#define TZX11GLOBALS_HPP

// X11 headers must come first so their include guards fire before any of our
// headers are parsed.
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h> // XImage, XLookupString, XSetWMProtocols
#include <X11/keysym.h>

// X11 event-type integer constants — stable X11 protocol values (see <X11/X.h>).
// We can't reuse the macro names as identifiers (the preprocessor would expand
// them before the compiler sees the declaration), so we use a k-prefix.
namespace X11ev {
static constexpr int kKeyPress = 2;
static constexpr int kKeyRelease = 3;
static constexpr int kButtonPress = 4;
static constexpr int kButtonRelease = 5;
static constexpr int kMotionNotify = 6;
static constexpr int kFocusIn = 9;
static constexpr int kFocusOut = 10;
static constexpr int kExpose = 12;
static constexpr int kConfigureNotify = 22;
static constexpr int kClientMessage = 33;
} // namespace X11ev

#undef None
#undef KeyPress
#undef KeyRelease
#undef ButtonPress
#undef ButtonRelease
#undef MotionNotify
#undef Expose
#undef ConfigureNotify
#undef ClientMessage
#undef FocusIn
#undef FocusOut
#ifdef Bool
#undef Bool
#endif
#ifdef True
#undef True
#endif
#ifdef False
#undef False
#endif

class TzX11Window;

class TzX11Globals
{
public:
    static TzX11Globals &instance();

    Display *display{nullptr};
    int screen{0};
    Visual *visual{nullptr};
    int depth{0};
    Window rootWin{0};

    // Atoms
    Atom wmDeleteWindow{0};
    Atom netWmName{0};
    Atom utf8String{0};

    int fd() const;
    void flush();

    void registerWindow(Window xwin, TzX11Window *win);
    void unregisterWindow(Window xwin);
    TzX11Window *windowForXWindow(Window xwin) const;

    void dispatchPendingEvents();

private:
    TzX11Globals();
    ~TzX11Globals();

    struct Entry
    {
        Window xwin;
        TzX11Window *win;
    };
    Entry m_windows[32];
    int m_count{0};
};

#endif // TZX11GLOBALS_HPP
