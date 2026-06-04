#include <loom/TzAbstractConsoleInput>
#include <loom/TzAbstractPlatformIntegration>
#include <loom/TzCoreApplication>
#include <loom/TzKeyEvent>
#include <loom/TzKeyboardHandler>
#include <loom/TzScopedPointer>
#include <loom/TzTimer>
#include <loom/TzLogging>
#include <memory>

int loom_main(int argc, char *argv[])
{
    TzCoreApplication app(argc, argv);

    auto consoleInput = tzScopedPointer(app.platformIntegration()->createConsoleInput());

    auto keyboard = tzScopedPointer(
        TzKeyboardHandler::create(consoleInput.get(), [&](TzKeyEvent *event) {
            if (event->key() == Key::Enter)
                tzInfo("Enter");
            else if (!event->utf8().empty())
                tzInfo("Text: {}", event->utf8());
            if (event->utf8() == "q")
                app.quit();
        }));

    auto periodic = tzScopedPointer(
        TzTimer::repeat(std::chrono::seconds(2), []() { tzInfo("Tick"); }));

    auto quit = tzScopedPointer(
        TzTimer::singleShot(std::chrono::seconds(5), [&]() { app.quit(); }));

    return app.exec();
}
