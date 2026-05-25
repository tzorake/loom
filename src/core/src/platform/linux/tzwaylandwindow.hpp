#ifndef TZWAYLANDWINDOW_HPP
#define TZWAYLANDWINDOW_HPP

#include "tzplatformwindow.hpp"

#include <memory>

class TzWaylandWindowPrivate;

class TzWaylandWindow : public TzPlatformWindow
{
public:
    TzWaylandWindow(int width, int height);
    virtual ~TzWaylandWindow() override;

    virtual void setTitle(const std::string &title) override;
    virtual void show() override;
    virtual void hide() override;

    virtual void render(const std::vector<uint32_t> &pixels, int width, int height) override;

private:
    std::unique_ptr<TzWaylandWindowPrivate> d_ptr;
};

#endif // TZWAYLANDWINDOW_HPP
