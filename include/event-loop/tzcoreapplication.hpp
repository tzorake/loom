#ifndef TZCOREAPPLICATION_HPP
#define TZCOREAPPLICATION_HPP

#include <event-loop/tzclasshelpermacros.hpp>

#include <memory>

class TzAbstractPlatformIntegration;
class TzAbstractEventDispatcher;
class TzCoreApplicationPrivate;

class TzCoreApplication
{
    TZ_DECLARE_PRIVATE(TzCoreApplication)
public:
    TzCoreApplication(int argc, char *argv[]);
    virtual ~TzCoreApplication();

    static TzCoreApplication *instance();

    TzAbstractPlatformIntegration *platformIntegration() const;
    TzAbstractEventDispatcher *eventDispatcher() const;

    int  exec();
    void quit(int exitCode = 0);

private:
    std::unique_ptr<TzCoreApplicationPrivate> d_ptr;
    static TzCoreApplication *s_instance;
};

#endif // TZCOREAPPLICATION_HPP
