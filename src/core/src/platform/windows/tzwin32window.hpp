#ifndef TZWIN32WINDOW_HPP
#define TZWIN32WINDOW_HPP

#include "tzplatformwindow.hpp"

#include <memory>

class TzWin32WindowPrivate;

class TzWin32Window : public TzPlatformWindow
{
public:
    TzWin32Window(int width, int height);
    virtual ~TzWin32Window() override;

    virtual void setTitle(const std::string &title) override;
    virtual void show() override;
    virtual void hide() override;

    virtual void render(const std::vector<uint32_t> &pixels, int width, int height) override;

    virtual TzNativeWindowHandle nativeWindowHandle() const override;

private:
    std::unique_ptr<TzWin32WindowPrivate> d_ptr;
};

#endif // TZWIN32WINDOW_HPP
