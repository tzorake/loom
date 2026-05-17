#ifndef TZWIN32EVENTDISPATCHER_HPP
#define TZWIN32EVENTDISPATCHER_HPP

#include <loom/tzabstracteventdispatcher.hpp>
#include <loom/tzclasshelpermacros.hpp>

#include <memory>

class TzWin32EventDispatcherPrivate;

class TzWin32EventDispatcher : public TzAbstractEventDispatcher
{
    TZ_DECLARE_PRIVATE(TzWin32EventDispatcher)
public:
    TzWin32EventDispatcher();
    virtual ~TzWin32EventDispatcher() override;

    virtual void processEvents() override;
    virtual void interrupt() override;
    virtual void wakeUp() override;
    virtual void setPreWaitCallback(PreWaitCallback callback) override;

    virtual TimerHandle registerTimer(TimerInterval interval, bool singleShot,
                                      TimerCallback callback) override;
    virtual void unregisterTimer(TimerHandle handle) override;

    virtual NotifyHandle registerSocketNotifier(int fd, NotifyCallback callback) override;
    virtual void unregisterSocketNotifier(NotifyHandle handle) override;

private:
    std::unique_ptr<TzWin32EventDispatcherPrivate> d_ptr;
};

#endif // TZWIN32EVENTDISPATCHER_HPP
