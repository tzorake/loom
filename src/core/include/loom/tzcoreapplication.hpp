#ifndef TZCOREAPPLICATION_HPP
#define TZCOREAPPLICATION_HPP

#include <loom/tzclasshelpermacros.hpp>

#include <memory>

class TzAbstractPlatformIntegration;
class TzAbstractEventDispatcher;
class TzCoreApplicationPrivate;
class TzObject;
class TzEvent;

#define tzApp TzCoreApplication::instance()

class TzCoreApplication
{
    TZ_DECLARE_PRIVATE(TzCoreApplication)
public:
    TzCoreApplication(int argc, char *argv[]);
    virtual ~TzCoreApplication();

    static TzCoreApplication *instance();

    TzAbstractPlatformIntegration *platformIntegration() const;
    TzAbstractEventDispatcher *eventDispatcher() const;

    int exec();
    void quit(int exitCode = 0);

    // Async: ownership of event transfers to the queue; delivered before next wait
    static void postEvent(TzObject *receiver, TzEvent *event);
    // Sync: calls receiver->event(event) immediately; caller retains ownership
    static bool sendEvent(TzObject *receiver, TzEvent *event);
    // Removes all queued events for receiver (called automatically from ~TzObject)
    void removePostedEvents(TzObject *receiver);

private:
    void processPostedEvents();

    std::unique_ptr<TzCoreApplicationPrivate> d_ptr;
    static TzCoreApplication *s_instance;
};

#endif // TZCOREAPPLICATION_HPP
