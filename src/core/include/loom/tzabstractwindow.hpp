#ifndef TZABSTRACTWINDOW_HPP
#define TZABSTRACTWINDOW_HPP

#include <event-loop/tzobject.hpp>
#include <event-loop/tzplatformsurface.hpp>
#include <event-loop/tzkeyevent.hpp>
#include <event-loop/tzmouseevent.hpp>
#include <event-loop/tzcloseevent.hpp>
#include <event-loop/tzresizeevent.hpp>

#include <string>

class TzAbstractWindowPrivate;

class TzAbstractWindow : public TzObject, public TzPlatformSurface
{
public:
    explicit TzAbstractWindow(TzObject *parent = nullptr);
    ~TzAbstractWindow() override;

    virtual void setTitle(const std::string &title) = 0;
    virtual void show() = 0;
    virtual void hide() = 0;

    // ── TzPlatformSurface ─────────────────────────────────────────────────
    TzSurface *surface() const override;
    void       setSurface(TzSurface *surface) override;

    void setCloseCallback (CloseCallback  cb) override;
    void setResizeCallback(ResizeCallback cb) override;
    void setKeyCallback   (KeyCallback    cb) override;
    void setMouseCallback (MouseCallback  cb) override;

    // render() remains pure virtual — implemented by platform subclass

    // ── TzObject ──────────────────────────────────────────────────────────
    bool event(TzEvent *event) override;

private:
    TzAbstractWindowPrivate *d_ptr;
};

#endif // TZABSTRACTWINDOW_HPP
