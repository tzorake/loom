#ifndef TZMACOSWINDOW_P_HPP
#define TZMACOSWINDOW_P_HPP

#include <event-loop/tzabstractwindow.hpp>
#include <event-loop/tzevent.hpp>
#include <event-loop/tzkeyevent.hpp>
#include <event-loop/tzmouseevent.hpp>
#include <event-loop/tzcoreapplication.hpp>

#include "tzobjcutils.hpp"

#include <CoreGraphics/CoreGraphics.h>

#include <cstdint>
#include <mutex>
#include <vector>

class TzMacosWindowPrivate
{
public:
    TzMacosWindowPrivate(int width, int height, TzAbstractWindow *owner);
    ~TzMacosWindowPrivate();

    void setTitle(const std::string& title);
    void show();
    void hide();
    void render(const std::vector<uint32_t>& pixels, int width, int height);

    // Called from ObjC callbacks
    bool onWindowShouldClose();
    void onWindowDidResize();
    void onDrawRect(ObjcObject self, CGRect rect);
    void onKeyEvent(ObjcObject nsEvent, bool pressed);
    void onMouseEvent(ObjcObject nsEvent, TzEvent::Type type, MouseButton button);

    TzAbstractWindow *owner{ nullptr };

    ObjcObject window{ nullptr };
    ObjcObject delegate{ nullptr };
    ObjcObject contentView{ nullptr };

    int windowWidth{ 0 };
    int windowHeight{ 0 };

    std::vector<uint32_t> pixels;
    int pixelWidth{ 0 };
    int pixelHeight{ 0 };
    std::mutex pixelMutex;

    // Close and resize are delivered synchronously: close must answer Cocoa
    // immediately; resize updates geometry before any dependent code runs.
    TzAbstractWindow::CloseCallback  closeCallback;
    TzAbstractWindow::ResizeCallback resizeCallback;
};

#endif // TZMACOSWINDOW_P_HPP
