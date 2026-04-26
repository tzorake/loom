#include <event-loop/TzMacosEventDispatcher>
#include <event-loop/TzEventLoop>
#include <event-loop/TzTimer>
#include <print>

int main()
{
    TzMacosEventDispatcher dispatcher;  
    TzEventLoop loop(&dispatcher);

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

    std::println("sum(2, 3) = {}", 2 + 3);

    loop.exec();

    return 0;
}
