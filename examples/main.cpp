#include <event-loop/TzMacosEventDispatcher>
#include <event-loop/TzEventLoop>
#include <event-loop/TzKeyboardHandler>
#include <event-loop/TzKeyEvent>
#include <event-loop/TzTimer>
#include <event-loop/TzScopedPointer>
#include <print>

int main()
{
    TzMacosEventDispatcher dispatcher;  
    TzEventLoop loop(&dispatcher);

    TzScopedPointer<TzKeyboardHandler> keyboard = TzKeyboardHandler::create(
        &dispatcher, [&](TzKeyEvent *event) {
            if (event->key == Key::Enter) std::println("Enter");
            else if (!event->utf8.empty()) std::println("Text: {}", event->utf8);
            if (event->utf8 == "q") loop.quit();
        });

    TzScopedPointer<TzTimer> periodic = TzTimer::repeat(
        &dispatcher, std::chrono::seconds(2), []() {
            std::println("Tick");
        });

    TzScopedPointer<TzTimer> quit = TzTimer::singleShot(
        &dispatcher, std::chrono::seconds(5), [&]() {
            loop.quit();
        });

    loop.exec();

    std::println("Event loop finished. Goodbye.");

    return 0;
}
