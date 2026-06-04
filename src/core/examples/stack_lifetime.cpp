#include <loom/TzCoreApplication>
#include <loom/TzTimer>
#include <loom/TzLogging>
#include <loom/TzScopedPointer>

struct Guard {
    const char *name;

    explicit Guard(const char *n) : name(n)
    {
        tzInfo("[{}] constructed", name);
    }

    ~Guard()
    {
        tzInfo("[{}] destructed", name);
    }

    void tick(int n) const
    {
        tzInfo("[{}] tick {} — still alive", name, n);
    }
};

int loom_main(int argc, char *argv[])
{
    TzCoreApplication app(argc, argv);
    Guard guard("Guard");

    int count = 0;

    auto timer = tzScopedPointer(TzTimer::repeat(std::chrono::seconds(1), [&] {
        guard.tick(++count);
        if (count >= 5) {
            tzInfo("quitting event loop");
            app.quit();
        }
    }));

    tzInfo("entering event loop");
    int rc = app.exec();

    tzInfo("event loop exited (rc={})", rc);
    return rc;
}
