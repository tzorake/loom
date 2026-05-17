#ifndef TZX11WINDOW_P_HPP
#define TZX11WINDOW_P_HPP

#include "tzx11globals.hpp"
#include "tzx11window.hpp"

#include <loom/tzcloseevent.hpp>
#include <loom/tzcoreapplication.hpp>
#include <loom/tzresizeevent.hpp>

#include <cstring>
#include <string>
#include <vector>

class TzX11WindowPrivate
{
public:
    TzX11WindowPrivate(int width, int height, TzX11Window *owner);
    ~TzX11WindowPrivate();

    void setTitle(const std::string &title);
    void show();
    void hide();
    void render(const std::vector<uint32_t> &pixels, int width, int height);
    void blitCache();

    TzX11Window *owner{nullptr};
    Display *display{nullptr};
    Window window{0};
    GC gc{nullptr};

    int windowWidth{0};
    int windowHeight{0};
    bool visible{false};

    // Pixel cache — last rendered frame, re-blitted on Expose.
    std::vector<char> pixelBuf;
    int lastRenderWidth{0};
    int lastRenderHeight{0};
};

#endif // TZX11WINDOW_P_HPP
