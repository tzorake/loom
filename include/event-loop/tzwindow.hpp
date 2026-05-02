#ifndef TZWINDOW_HPP
#define TZWINDOW_HPP

#include <event-loop/tzobject.hpp>
#include <event-loop/tzclasshelpermacros.hpp>

#include <functional>
#include <string>

class TzWidget;
class TzScene;
class TzWindowPrivate;

class TzWindow : public TzObject
{
    TZ_DECLARE_PRIVATE_D(d_ptr, TzWindow)
public:
    explicit TzWindow(TzWindow *parent = nullptr);
    TzWindow(int width, int height, TzWindow *parent = nullptr);
    ~TzWindow() override;

    void setTitle(const std::string &title);
    void show();
    void hide();
    void close();

    TzScene *scene();
    void setRootWidget(TzWidget *root);
    TzWidget *rootWidget() const;

    int width() const;
    int height() const;

    void setOnClose(std::function<void()> cb);

protected:
    explicit TzWindow(TzWindowPrivate &d, int width, int height, TzWindow *parent = nullptr);

    virtual void closeEvent();
};

#endif // TZWINDOW_HPP
