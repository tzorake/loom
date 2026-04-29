#ifndef TZMACOSWINDOW_P_HPP
#define TZMACOSWINDOW_P_HPP

#include <event-loop/tzabstractwindow.hpp>

#include "tzobjcutils.hpp"

#include <CoreGraphics/CoreGraphics.h>

#include <cstdint>
#include <mutex>
#include <vector>

class TzMacosWindowPrivate
{
public:
    TzMacosWindowPrivate(int width, int height);
    ~TzMacosWindowPrivate();

    void setTitle(const std::string& title);
    void show();
    void hide();
    void render(const std::vector<uint32_t>& pixels, int width, int height);

    // Called from ObjC callbacks
    bool onWindowShouldClose();
    void onWindowDidResize();
    void onDrawRect(ObjcObject self, CGRect rect);

    ObjcObject window{ nullptr };
    ObjcObject delegate{ nullptr };
    ObjcObject contentView{ nullptr };

    int windowWidth{ 0 };
    int windowHeight{ 0 };

    std::vector<uint32_t> pixels;
    int pixelWidth{ 0 };
    int pixelHeight{ 0 };
    std::mutex pixelMutex;

    TzAbstractWindow::CloseCallback  closeCallback;
    TzAbstractWindow::ResizeCallback resizeCallback;
};

#endif // TZMACOSWINDOW_P_HPP
