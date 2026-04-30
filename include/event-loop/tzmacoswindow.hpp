#ifndef TZMACOSWINDOW_HPP
#define TZMACOSWINDOW_HPP

#include <event-loop/tzabstractwindow.hpp>

#include <memory>

class TzMacosWindowPrivate;

class TzMacosWindow : public TzAbstractWindow
{
public:
    TzMacosWindow(int width, int height);
    virtual ~TzMacosWindow() override;

    virtual void setTitle(const std::string& title) override;
    virtual void show() override;
    virtual void hide() override;

    virtual void setCloseCallback(CloseCallback callback) override;
    virtual void setResizeCallback(ResizeCallback callback) override;
    virtual void setKeyCallback(KeyCallback callback) override;
    virtual void setMouseCallback(MouseCallback callback) override;

    virtual void render(const std::vector<uint32_t>& pixels, int width, int height) override;

private:
    std::unique_ptr<TzMacosWindowPrivate> d_ptr;
};

#endif // TZMACOSWINDOW_HPP
