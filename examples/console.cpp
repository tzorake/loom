#include <event-loop/TzCoreApplication>
#include <event-loop/TzAbstractPlatformIntegration>
#include <event-loop/TzAbstractConsoleInput>
#include <event-loop/TzKeyboardHandler>
#include <event-loop/TzKeyEvent>
#include <event-loop/TzTimer>
#include <event-loop/TzScopedPointer>
#include <print>
#include <memory>

int main(int argc, char *argv[])
{
    TzCoreApplication app(argc, argv);

    auto consoleInput = tz::as_scoped_ptr(app.platformIntegration()->createConsoleInput());

    auto keyboard = tz::as_scoped_ptr(TzKeyboardHandler::create(
        app.eventDispatcher(), consoleInput.get(),
        [&](TzKeyEvent *event) {
            if (event->key == Key::Enter) std::println("Enter");
            else if (!event->utf8.empty()) std::println("Text: {}", event->utf8);
            if (event->utf8 == "q") app.quit();
        }));

    auto periodic = tz::as_scoped_ptr(
        TzTimer::repeat(app.eventDispatcher(), std::chrono::seconds(2),
            []() { std::println("Tick"); }));

    auto quit = tz::as_scoped_ptr(
        TzTimer::singleShot(app.eventDispatcher(), std::chrono::seconds(5),
            [&]() { app.quit(); }));

    return app.exec();
}
