#ifndef TZWINDOWSEVENTDISPATCHER_HPP
#define TZWINDOWSEVENTDISPATCHER_HPP

#include <loom/tzclasshelpermacros.hpp>
#include <loom/tzabstracteventdispatcher.hpp>

#include <memory>

class TzWindowsEventDispatcherPrivate;

class TzWindowsEventDispatcher : public TzAbstractEventDispatcher
{
    TZ_DECLARE_PRIVATE(TzWindowsEventDispatcher)
public:
    TzWindowsEventDispatcher();
    virtual ~TzWindowsEventDispatcher() override;

    virtual void processEvents() override;
    virtual void interrupt() override;
    virtual void wakeUp() override;
    virtual void setPreWaitCallback(PreWaitCallback callback) override;

    virtual TimerHandle registerTimer(TimerInterval interval, bool singleShot, TimerCallback callback) override;
    virtual void unregisterTimer(TimerHandle handle) override;

    virtual NotifyHandle registerSocketNotifier(int fd, NotifyCallback callback) override;
    virtual void unregisterSocketNotifier(NotifyHandle handle) override;

private:
    std::unique_ptr<TzWindowsEventDispatcherPrivate> d_ptr;
};

#endif // TZWINDOWSEVENTDISPATCHER_HPP
