#ifndef TZX11EVENTDISPATCHER_HPP
#define TZX11EVENTDISPATCHER_HPP

#include <loom/tzclasshelpermacros.hpp>
#include <loom/tzabstracteventdispatcher.hpp>

#include <memory>

class TzX11EventDispatcherPrivate;

class TzX11EventDispatcher : public TzAbstractEventDispatcher
{
    TZ_DECLARE_PRIVATE(TzX11EventDispatcher)
public:
    TzX11EventDispatcher();
    ~TzX11EventDispatcher() override;

    void processEvents() override;
    void interrupt()     override;
    void wakeUp()        override;
    void setPreWaitCallback(PreWaitCallback callback) override;

    TimerHandle  registerTimer(TimerInterval interval, bool singleShot,
                               TimerCallback callback) override;
    void         unregisterTimer(TimerHandle handle) override;

    NotifyHandle registerSocketNotifier(int fd, NotifyCallback callback) override;
    void         unregisterSocketNotifier(NotifyHandle handle) override;

    // Called by the platform integration once the X11 display is available.
    void setX11Fd(int fd);

private:
    std::unique_ptr<TzX11EventDispatcherPrivate> d_ptr;
};

#endif // TZX11EVENTDISPATCHER_HPP
