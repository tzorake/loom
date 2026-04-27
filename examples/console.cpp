#include <event-loop/TzAbstractPlatformIntegration>
#include <event-loop/TzAbstractEventDispatcher>
#include <event-loop/TzAbstractConsoleInput>
#include <event-loop/TzEventLoop>
#include <event-loop/TzKeyboardHandler>
#include <event-loop/TzKeyEvent>
#include <event-loop/TzTimer>
#include <event-loop/TzScopedPointer>
#include <print>
#include <memory>

int main()
{
    auto platform = tz::as_scoped_ptr(createPlatformIntegration());
    auto dispatcher = tz::as_scoped_ptr(platform->createEventDispatcher());
    auto consoleInput = tz::as_scoped_ptr(platform->createConsoleInput());

    TzEventLoop loop(dispatcher.get());

    auto onKeyboardEvent = [&](TzKeyEvent *event) {
        if (event->key == Key::Enter) std::println("Enter");
        else if (!event->utf8.empty()) std::println("Text: {}", event->utf8);
        if (event->utf8 == "q") loop.quit();
    };
    auto keyboard = tz::as_scoped_ptr(
        TzKeyboardHandler::create(dispatcher.get(), consoleInput.get(), onKeyboardEvent));

    auto onRepeat = []() { std::println("Tick"); };
    auto periodic = tz::as_scoped_ptr(
        TzTimer::repeat(dispatcher.get(), std::chrono::seconds(2), onRepeat));
    
    auto onSingleShot = [&]() { loop.quit(); };
    auto quit = tz::as_scoped_ptr(
        TzTimer::singleShot(dispatcher.get(), std::chrono::seconds(5), onSingleShot));

    loop.exec();

    std::println("Event loop finished. Goodbye.");

    return 0;
}
