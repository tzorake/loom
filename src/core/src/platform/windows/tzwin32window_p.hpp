#ifndef TZWIN32WINDOW_P_HPP
#define TZWIN32WINDOW_P_HPP

#include "tzwin32window.hpp"
#include <loom/tzcloseevent.hpp>
#include <loom/tzcoreapplication.hpp>
#include <loom/tzkeyevent.hpp>
#include <loom/tzmouseevent.hpp>
#include <loom/tzresizeevent.hpp>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <cstdint>
#include <mutex>
#include <vector>

class TzWin32WindowPrivate
{
public:
    TzWin32WindowPrivate(int width, int height, TzAbstractWindow *owner);
    ~TzWin32WindowPrivate();

    void setTitle(const std::string &title);
    void show();
    void hide();
    void render(const std::vector<uint32_t> &pixels, int width, int height);

    void onClose();
    void onSize(int width, int height);
    void onPaint();
    void onKey(WPARAM vk, bool pressed, LPARAM lParam);
    void onMouseButton(UINT msg, WPARAM wParam, LPARAM lParam);
    void onMouseMove(LPARAM lParam, WPARAM wParam);
    void onMouseWheel(UINT msg, WPARAM wParam, LPARAM lParam);

    TzAbstractWindow *owner{nullptr};
    HWND hwnd{nullptr};
    int windowWidth{0};
    int windowHeight{0};

    std::vector<uint32_t> pixels;
    int pixelWidth{0};
    int pixelHeight{0};
    std::mutex pixelMutex;

    static LRESULT CALLBACK wndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
};

#endif // TZWIN32WINDOW_P_HPP
