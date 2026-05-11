#include <loom/TzGuiApplication>
#include <loom/TzAbstractWindow>
#include <loom/TzKeyEvent>
#include <loom/TzMouseEvent>
#include <loom/TzTimer>
#include <loom/TzScopedPointer>

#include <cmath>
#include <cstdint>
#include <print>
#include <vector>

static constexpr int kFps = 60;

// Persistent buffers — allocated once, resized only when window dimensions change.
static std::vector<uint32_t> s_pixels;
static std::vector<float>    s_cosR;   // per column:   cos(x/w + t)
static std::vector<float>    s_sinG;   // per row:      sin(y/h + t)
static std::vector<float>    s_cosB;   // per diagonal: cos((x+y)/(w+h) + t)

// Generates the animated frame into s_pixels.
//
// Original cost: 3 * w * h  trig calls  (~1.44 M at 800×600 → ~30 ms).
// Optimised cost: w + h + (w+h) trig calls (~2 200 at 800×600 → <0.1 ms)
// because r depends only on x, g only on y, and b only on (x+y).
//
// Pixels are stored bottom-up so render()'s row-flip produces a correct
// top-down image on screen.
static void generateFrame(std::size_t frameId, int width, int height)
{
    const size_t total = static_cast<size_t>(width) * static_cast<size_t>(height);
    s_pixels.resize(total);
    s_cosR.resize(static_cast<size_t>(width));
    s_sinG.resize(static_cast<size_t>(height));
    s_cosB.resize(static_cast<size_t>(width + height));

    const double t = static_cast<double>(frameId) / kFps;

    for (int x = 0; x < width; ++x)
        s_cosR[static_cast<size_t>(x)] =
            static_cast<float>(std::cos(static_cast<double>(x) / width + t) * 0.5 + 0.5);

    for (int y = 0; y < height; ++y)
        s_sinG[static_cast<size_t>(y)] =
            static_cast<float>(std::sin(static_cast<double>(y) / height + t) * 0.5 + 0.5);

    for (int k = 0; k < width + height; ++k)
        s_cosB[static_cast<size_t>(k)] =
            static_cast<float>(std::cos(static_cast<double>(k) / (width + height) + t) * 0.5 + 0.5);

    // render() expects bottom-up rows (row 0 = bottom), so write row y at
    // position (height-1-y) so the flip inside render() produces a correct
    // top-down image.
    for (int y = 0; y < height; ++y) {
        const auto  g   = static_cast<uint32_t>(s_sinG[static_cast<size_t>(y)] * 255.0f);
        uint32_t   *row = s_pixels.data() +
                          static_cast<size_t>(height - 1 - y) * static_cast<size_t>(width);
        for (int x = 0; x < width; ++x) {
            const auto r = static_cast<uint32_t>(s_cosR[static_cast<size_t>(x)]       * 255.0f);
            const auto b = static_cast<uint32_t>(s_cosB[static_cast<size_t>(x + y)]   * 255.0f);
            row[static_cast<size_t>(x)] = (0xFFu << 24) | (r << 16) | (g << 8) | b;
        }
    }
}

int main(int argc, char *argv[])
{
    TzGuiApplication app(argc, argv);

    int renderWidth  = 800;
    int renderHeight = 600;

    auto window = tz::as_scoped_ptr(app.createWindow(renderWidth, renderHeight));
    window->setTitle("event-loop window");
    window->setCloseCallback([&]() { app.quit(); });
    window->setKeyCallback([&](TzKeyEvent *event) {
        if (event->key() == Key::Escape) app.quit();
        else if (!event->utf8().empty()) std::println("Key: {}", event->utf8());
    });
    window->setMouseCallback([](TzMouseEvent *event) {
        switch (event->type()) {
            case TzEvent::MouseButtonPress:
                std::println("Mouse press   ({:.0f}, {:.0f}) button={}",
                    event->x(), event->y(), (int)event->button());
                break;
            case TzEvent::MouseButtonRelease:
                std::println("Mouse release ({:.0f}, {:.0f}) button={}",
                    event->x(), event->y(), (int)event->button());
                break;
            case TzEvent::MouseScroll:
                std::println("Scroll dx={:.1f} dy={:.1f}", event->scrollDx(), event->scrollDy());
                break;
            default: break;
        }
    });

    // Declare frame before the resize callback so the lambda can capture it.
    std::size_t frame = 0;

    window->setResizeCallback([&](int w, int h) {
        renderWidth  = w;
        renderHeight = h;
        std::println("Resized: {}x{}", w, h);
        // Render immediately so the compositor gets a correctly-sized frame
        // right after the resize event rather than waiting for the next tick.
        generateFrame(frame, renderWidth, renderHeight);
        window->render(s_pixels, renderWidth, renderHeight);
    });

    window->show();

    auto ticker = tz::as_scoped_ptr(
        TzTimer::repeat(app.eventDispatcher(), std::chrono::milliseconds(1000 / kFps),
            [&]() {
                generateFrame(frame++, renderWidth, renderHeight);
                window->render(s_pixels, renderWidth, renderHeight);
            }));

    return app.exec();
}
