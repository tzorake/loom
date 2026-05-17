#ifndef TZCOCOAWINDOW_HPP
#define TZCOCOAWINDOW_HPP

#include <loom/tzabstractwindow.hpp>

#include <memory>

class TzCocoaWindowPrivate;

class TzCocoaWindow : public TzAbstractWindow
{
public:
    TzCocoaWindow(int width, int height);
    virtual ~TzCocoaWindow() override;

    virtual void setTitle(const std::string &title) override;
    virtual void show() override;
    virtual void hide() override;

    virtual void render(const std::vector<uint32_t> &pixels, int width, int height) override;

private:
    std::unique_ptr<TzCocoaWindowPrivate> d_ptr;
};

#endif // TZCOCOAWINDOW_HPP
