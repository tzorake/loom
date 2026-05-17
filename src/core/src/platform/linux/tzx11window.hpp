#ifndef TZX11WINDOW_HPP
#define TZX11WINDOW_HPP

#include <loom/tzabstractwindow.hpp>

#include <memory>
#include <string>
#include <vector>

class TzX11WindowPrivate;

class TzX11Window : public TzAbstractWindow
{
public:
    TzX11Window(int width, int height);
    ~TzX11Window() override;

    void setTitle(const std::string &title) override;
    void show() override;
    void hide() override;
    void render(const std::vector<uint32_t> &pixels, int width, int height) override;

    // Called by TzX11Globals during event dispatch.
    void onConfigure(int width, int height);
    void onExpose();

private:
    std::unique_ptr<TzX11WindowPrivate> d_ptr;
};

#endif // TZX11WINDOW_HPP
