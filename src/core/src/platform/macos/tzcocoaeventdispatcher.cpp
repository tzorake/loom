#include "tzcocoaeventdispatcher.hpp"

#include "tzcocoaeventdispatcher_p.hpp"
#include "tzobjcutils.hpp"

TzCocoaEventDispatcherPrivate::TzCocoaEventDispatcherPrivate()
    : runLoop(CFRunLoopGetCurrent())
{
    CFRetain(runLoop);
}

TzCocoaEventDispatcher::TzCocoaEventDispatcher()
    : d_ptr(new TzCocoaEventDispatcherPrivate)
{
    d_ptr->q_ptr = this;
}

TzCocoaEventDispatcher::~TzCocoaEventDispatcher()
{
    while (!d_ptr->timerMap.empty())
        unregisterTimer(d_ptr->timerMap.begin()->first);
    if (d_ptr->preWaitObserver) {
        CFRunLoopRemoveObserver(d_ptr->runLoop, d_ptr->preWaitObserver, kCFRunLoopCommonModes);
        CFRelease(d_ptr->preWaitObserver);
    }
    CFRelease(d_ptr->runLoop);
}

void TzCocoaEventDispatcher::processEvents()
{
    d_ptr->interrupted = false;

    // Use NSApplication when Cocoa is loaded (windowed apps), otherwise CFRunLoop.
    ObjcClass appClass = getClass("NSApplication");
    if (appClass) {
        ObjcObject app = sendClassMessage<ObjcObject>(appClass, "sharedApplication");
        sendMessage<void>(app, "run");
    } else {
        CFRunLoopRun();
    }
}

void TzCocoaEventDispatcher::interrupt()
{
    d_ptr->interrupted = true;

    ObjcClass appClass = getClass("NSApplication");
    if (appClass) {
        ObjcObject app = sendClassMessage<ObjcObject>(appClass, "sharedApplication");
        sendMessage<void>(app, "stop:", nullptr);

        // [NSApp stop:] only takes effect after the next event; post a dummy
        // NSApplicationDefined event (type 15) so the run loop unblocks immediately.
        using NSUInteger = unsigned long;
        using NSInteger  = long;
        CGPoint zero{};
        ObjcObject event = reinterpret_cast<ObjcObject(*)(
            ObjcClass, objc_selector*,
            NSUInteger, CGPoint, NSUInteger, double,
            NSInteger, ObjcObject, short, NSInteger, NSInteger)>(objc_msgSend)(
            getClass("NSEvent"),
            sel_registerName("otherEventWithType:location:modifierFlags:timestamp:"
                             "windowNumber:context:subtype:data1:data2:"),
            (NSUInteger)15, zero, (NSUInteger)0, (double)0.0,
            (NSInteger)0, nullptr, (short)0, (NSInteger)0, (NSInteger)0
        );
        sendMessage<void>(app, "postEvent:atStart:", event, (int)YES);
    } else {
        CFRunLoopStop(d_ptr->runLoop);
    }
}

void TzCocoaEventDispatcher::wakeUp()
{
    CFRunLoopWakeUp(d_ptr->runLoop);
}

static void preWaitObserverCallback(CFRunLoopObserverRef, CFRunLoopActivity, void *info)
{
    auto *d = static_cast<TzCocoaEventDispatcherPrivate *>(info);
    if (d->preWaitCallback)
        d->preWaitCallback();
}

void TzCocoaEventDispatcher::setPreWaitCallback(PreWaitCallback callback)
{
    d_ptr->preWaitCallback = std::move(callback);

    if (d_ptr->preWaitObserver)
        return;

    CFRunLoopObserverContext ctx{
        .version = 0,
        .info = d_ptr.get(),
        .retain = nullptr,
        .release = nullptr,
        .copyDescription = nullptr
    };
    d_ptr->preWaitObserver = CFRunLoopObserverCreate(
        kCFAllocatorDefault,
        kCFRunLoopBeforeWaiting,
        true,
        0,
        preWaitObserverCallback,
        &ctx
    );
    CFRunLoopAddObserver(d_ptr->runLoop, d_ptr->preWaitObserver, kCFRunLoopCommonModes);
}

static void timerCallback(CFRunLoopTimerRef timer, void *info)
{
    auto *wrapper = static_cast<TzCocoaEventDispatcherPrivate::TimerWrapper *>(info);
    if (!wrapper || !wrapper->callback)
        return;

    wrapper->callback();

    if (wrapper->singleShot)
        wrapper->eventDispatcher->unregisterTimer(static_cast<TzAbstractEventDispatcher::TimerHandle>(wrapper));
}

TzCocoaEventDispatcher::TimerHandle TzCocoaEventDispatcher::registerTimer(TimerInterval interval, bool singleShot, TimerCallback callback)
{
    CFTimeInterval secs = interval.count() / 1000.0;
    CFAbsoluteTime firstFire = CFAbsoluteTimeGetCurrent() + secs;

    auto wrapper = std::make_unique<TzCocoaEventDispatcherPrivate::TimerWrapper>();
    wrapper->callback = std::move(callback);
    wrapper->singleShot = singleShot;
    wrapper->eventDispatcher = this;

    CFRunLoopTimerContext context{
        .version = 0, 
        .info = wrapper.get(), 
        .retain = nullptr, 
        .release = nullptr, 
        .copyDescription = nullptr
    };
    CFRunLoopTimerRef timer = CFRunLoopTimerCreate(
        kCFAllocatorDefault,
        firstFire,
        singleShot ? 0.0 : secs,
        0,
        0,
        timerCallback,
        &context
    );
    if (!timer)
        throw std::runtime_error("CFRunLoopTimerCreate");

    CFRunLoopAddTimer(d_ptr->runLoop, timer, kCFRunLoopCommonModes);
    wrapper->timer = timer;

    TzCocoaEventDispatcher::TimerHandle handle = static_cast<TzCocoaEventDispatcher::TimerHandle>(wrapper.get());
    d_ptr->timerMap[handle] = std::move(wrapper);
    return handle;
}

void TzCocoaEventDispatcher::unregisterTimer(TimerHandle handle)
{
    auto it = d_ptr->timerMap.find(handle);
    if (it == d_ptr->timerMap.end())
        return;
    
    if (it->second->timer) {
        CFRunLoopRemoveTimer(d_ptr->runLoop, it->second->timer, kCFRunLoopCommonModes);
        CFRelease(it->second->timer);
    }
    d_ptr->timerMap.erase(it);
}

static void notifyCallback(CFSocketRef s, CFSocketCallBackType type, CFDataRef address, const void *data, void *info)
{
    (void)address; (void)data;
    auto *wrapper = static_cast<TzCocoaEventDispatcherPrivate::NotifyWrapper *>(info);
    if (!wrapper || !wrapper->callback)
        return;

    if (type == kCFSocketReadCallBack) {
        CFSocketNativeHandle fd = CFSocketGetNative(s);
        wrapper->callback(static_cast<int>(fd));
    }
}

TzCocoaEventDispatcher::NotifyHandle TzCocoaEventDispatcher::registerSocketNotifier(int fd, NotifyCallback callback)
{
    auto wrapper = std::make_unique<TzCocoaEventDispatcherPrivate::NotifyWrapper>();
    wrapper->callback = std::move(callback);
    wrapper->eventDispatcher = this;

    CFSocketContext context{
        .version = 0,
        .info = wrapper.get(),
        .retain = nullptr,
        .release = nullptr,
        .copyDescription = nullptr
    };
    CFSocketRef socket = CFSocketCreateWithNative(
        kCFAllocatorDefault,
        fd,
        kCFSocketReadCallBack,
        notifyCallback,
        &context
    );
    if (!socket)
        throw std::runtime_error("CFSocketCreateWithNative");

    CFRunLoopSourceRef source = CFSocketCreateRunLoopSource(kCFAllocatorDefault, socket, 0);
    if (!source) {
        CFRelease(socket);
        throw std::runtime_error("CFSocketCreateRunLoopSource");
    }
    CFRunLoopAddSource(d_ptr->runLoop, source, kCFRunLoopCommonModes);

    wrapper->socket = socket;
    wrapper->source = source;

    NotifyHandle handle = static_cast<NotifyHandle>(wrapper.get());
    d_ptr->notifyMap[handle] = std::move(wrapper);
    return handle;
}

void TzCocoaEventDispatcher::unregisterSocketNotifier(NotifyHandle handle)
{
    auto it = d_ptr->notifyMap.find(handle);
    if (it == d_ptr->notifyMap.end())
        return;

    if (it->second->source) {
        CFRunLoopRemoveSource(d_ptr->runLoop, it->second->source, kCFRunLoopCommonModes);
        CFRelease(it->second->source);
    }
    if (it->second->socket) {
        CFSocketInvalidate(it->second->socket);
        CFRelease(it->second->socket);
    }
    d_ptr->notifyMap.erase(it);
}
