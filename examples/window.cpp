#include <event-loop/TzAbstractPlatformIntegration>
#include <event-loop/TzAbstractEventDispatcher>
#include <event-loop/TzAbstractConsoleInput>
#include <event-loop/TzAbstractWindow>
#include <event-loop/TzEventLoop>
#include <event-loop/TzTimer>
#include <event-loop/TzScopedPointer>

#include <cmath>
#include <cstdint>
#include <print>
#include <vector>

static constexpr int kWidth  = 800;
static constexpr int kHeight = 600;
static constexpr int kFps    = 60;

static std::vector<uint32_t> generateFrame(std::size_t frameId)
{
    double t = frameId / (double)kFps;
    std::vector<uint32_t> pixels(kWidth * kHeight);
    for (int y = 0; y < kHeight; ++y) {
        for (int x = 0; x < kWidth; ++x) {
            uint8_t r = (uint8_t)((std::cos((double)x / kWidth  + t) * 0.5 + 0.5) * 255);
            uint8_t g = (uint8_t)((std::sin((double)y / kHeight + t) * 0.5 + 0.5) * 255);
            uint8_t b = (uint8_t)((std::cos((double)(x + y) / (kWidth + kHeight) + t) * 0.5 + 0.5) * 255);
            pixels[y * kWidth + x] = (0xFFu << 24) | (r << 16) | (g << 8) | b;
        }
    }
    return pixels;
}

int main()
{
    auto platform = tz::as_scoped_ptr(createPlatformIntegration());
    auto dispatcher = tz::as_scoped_ptr(platform->createEventDispatcher());
    auto window = tz::as_scoped_ptr(platform->createWindow(kWidth, kHeight));

    TzEventLoop loop(dispatcher.get());

    window->setTitle("event-loop window");
    window->setCloseCallback([&]() { loop.quit(); });
    window->show();

    std::size_t frame = 0;
    auto ticker = tz::as_scoped_ptr(
        TzTimer::repeat(dispatcher.get(), std::chrono::milliseconds(1000 / kFps), 
            [&]() { window->render(generateFrame(frame++), kWidth, kHeight); }));

    loop.exec();

    std::println("Window closed.");

    return 0;
}
