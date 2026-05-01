#ifndef TZABSTRACTWINDOW_HPP
#define TZABSTRACTWINDOW_HPP

#include <event-loop/tzobject.hpp>
#include <event-loop/tzkeyevent.hpp>
#include <event-loop/tzmouseevent.hpp>

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

class TzAbstractWindowPrivate;

class TzAbstractWindow : public TzObject
{
public:
    using CloseCallback  = std::function<void()>;
    using ResizeCallback = std::function<void(int width, int height)>;
    using KeyCallback    = std::function<void(TzKeyEvent *)>;
    using MouseCallback  = std::function<void(TzMouseEvent *)>;

    explicit TzAbstractWindow(TzObject *parent = nullptr);
    virtual ~TzAbstractWindow() override;

    virtual void setTitle(const std::string &title) = 0;
    virtual void show() = 0;
    virtual void hide() = 0;

    virtual void setCloseCallback(CloseCallback callback)   = 0;
    virtual void setResizeCallback(ResizeCallback callback) = 0;

    void setKeyCallback(KeyCallback callback);
    void setMouseCallback(MouseCallback callback);

    virtual void render(const std::vector<uint32_t> &pixels, int width, int height) = 0;

    bool event(TzEvent *event) override;

private:
    std::unique_ptr<TzAbstractWindowPrivate> d_ptr;
};

#endif // TZABSTRACTWINDOW_HPP
