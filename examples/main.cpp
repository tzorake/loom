#include <event-loop/TzMacosEventDispatcher>
#include <event-loop/TzEventLoop>
#include <event-loop/TzKeyboardHandler>
#include <event-loop/TzKeyEvent>
#include <event-loop/TzTimer>
#include <print>

int main()
{
    TzMacosEventDispatcher dispatcher;  
    TzEventLoop loop(&dispatcher);

    TzKeyboardHandler keyboard(&dispatcher);
    keyboard.setCallback([&](TzKeyEvent *event) {
        if (event->key == Key::Enter) std::println("Enter");
        else if (!event->utf8.empty()) std::println("Text: {}", event->utf8);
        if (event->utf8 == "q") loop.quit();
    });
    keyboard.start();

    TzTimer periodic(&dispatcher);
    periodic.setInterval(std::chrono::seconds(2));
    periodic.setSingleShot(false);
    periodic.setCallback([]() { std::println("Tick"); });
    periodic.start();

    TzTimer quit(&dispatcher);
    quit.setInterval(std::chrono::seconds(5));
    quit.setSingleShot(true);
    quit.setCallback([&]() { loop.quit(); });
    quit.start();

    loop.exec();

    std::println("Terminate");

    return 0;
}
