#ifndef TZCOCOAWINDOW_P_HPP
#define TZCOCOAWINDOW_P_HPP

#include <loom/tzcloseevent.hpp>
#include <loom/tzcoreapplication.hpp>
#include <loom/tzevent.hpp>
#include <loom/tzkeyevent.hpp>
#include <loom/tzmouseevent.hpp>
#include <loom/tzresizeevent.hpp>

#include "tzobjcutils.hpp"

#include <CoreGraphics/CoreGraphics.h>

#include <cstdint>
#include <mutex>
#include <vector>

class TzCocoaWindowPrivate
{
public:
    TzCocoaWindowPrivate(int width, int height, TzPlatformWindow *owner);
    ~TzCocoaWindowPrivate();

    void setTitle(const std::string &title);
    void show();
    void hide();
    void render(const std::vector<uint32_t> &pixels, int width, int height);

    // Called from ObjC callbacks
    bool onWindowShouldClose();
    void onWindowDidResize();
    void onDrawRect(ObjcObject self, CGRect rect);
    void onKeyEvent(ObjcObject nsEvent, bool pressed);
    void onMouseEvent(ObjcObject nsEvent, TzEvent::Type type, MouseButton button);

    TzPlatformWindow *owner{nullptr};

    ObjcObject window{nullptr};
    ObjcObject delegate{nullptr};
    ObjcObject contentView{nullptr};

    int windowWidth{0};
    int windowHeight{0};

    std::vector<uint32_t> pixels;
    int pixelWidth{0};
    int pixelHeight{0};
    std::mutex pixelMutex;
};

#endif // TZCOCOAWINDOW_P_HPP
