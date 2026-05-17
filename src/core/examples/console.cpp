#include <loom/TzAbstractConsoleInput>
#include <loom/TzAbstractPlatformIntegration>
#include <loom/TzCoreApplication>
#include <loom/TzKeyEvent>
#include <loom/TzKeyboardHandler>
#include <loom/TzScopedPointer>
#include <loom/TzTimer>
#include <memory>
#include <print>

int main(int argc, char *argv[])
{
    TzCoreApplication app(argc, argv);

    auto consoleInput = tz::as_scoped_ptr(app.platformIntegration()->createConsoleInput());

    auto keyboard = tz::as_scoped_ptr(
        TzKeyboardHandler::create(app.eventDispatcher(), consoleInput.get(), [&](TzKeyEvent *event) {
            if (event->key() == Key::Enter)
                std::println("Enter");
            else if (!event->utf8().empty())
                std::println("Text: {}", event->utf8());
            if (event->utf8() == "q")
                app.quit();
        }));

    auto periodic = tz::as_scoped_ptr(TzTimer::repeat(app.eventDispatcher(), std::chrono::seconds(2),
                                                      []() { std::println("Tick"); }));

    auto quit = tz::as_scoped_ptr(
        TzTimer::singleShot(app.eventDispatcher(), std::chrono::seconds(5), [&]() { app.quit(); }));

    return app.exec();
}
