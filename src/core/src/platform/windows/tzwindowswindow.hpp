#ifndef TZWINDOWSWINDOW_HPP
#define TZWINDOWSWINDOW_HPP

#include <loom/tzabstractwindow.hpp>

#include <memory>

class TzWindowsWindowPrivate;

class TzWindowsWindow : public TzAbstractWindow
{
public:
    TzWindowsWindow(int width, int height);
    virtual ~TzWindowsWindow() override;

    virtual void setTitle(const std::string &title) override;
    virtual void show() override;
    virtual void hide() override;

    virtual void render(const std::vector<uint32_t> &pixels, int width, int height) override;

private:
    std::unique_ptr<TzWindowsWindowPrivate> d_ptr;
};

#endif // TZWINDOWSWINDOW_HPP
