#ifndef TZCOCOAEVENTDISPATCHER_HPP
#define TZCOCOAEVENTDISPATCHER_HPP

#include <loom/tzclasshelpermacros.hpp>
#include <loom/tzabstracteventdispatcher.hpp>

#include <memory>

class TzCocoaEventDispatcherPrivate;

class TzCocoaEventDispatcher : public TzAbstractEventDispatcher
{
    TZ_DECLARE_PRIVATE(TzCocoaEventDispatcher)
public:
    TzCocoaEventDispatcher();
    virtual ~TzCocoaEventDispatcher() override;

    virtual void processEvents() override;
    virtual void interrupt() override;
    virtual void wakeUp() override;
    virtual void setPreWaitCallback(PreWaitCallback callback) override;

    virtual TimerHandle registerTimer(TimerInterval interval, bool singleShot, TimerCallback callback) override;
    virtual void unregisterTimer(TimerHandle) override;

    virtual NotifyHandle registerSocketNotifier(int fd, NotifyCallback callback) override;
    virtual void unregisterSocketNotifier(NotifyHandle handle) override;

private:
    std::unique_ptr<TzCocoaEventDispatcherPrivate> d_ptr;
};

#endif // TZCOCOAEVENTDISPATCHER_HPP
