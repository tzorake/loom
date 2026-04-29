#ifndef TZABSTRACTWINDOW_HPP
#define TZABSTRACTWINDOW_HPP

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

class TzAbstractWindow
{
public:
    using CloseCallback  = std::function<void()>;
    using ResizeCallback = std::function<void(int width, int height)>;

    virtual ~TzAbstractWindow();

    virtual void setTitle(const std::string& title) = 0;
    virtual void show() = 0;
    virtual void hide() = 0;

    virtual void setCloseCallback(CloseCallback callback) = 0;
    virtual void setResizeCallback(ResizeCallback callback) = 0;

    virtual void render(const std::vector<uint32_t>& pixels, int width, int height) = 0;
};

#endif // TZABSTRACTWINDOW_HPP
