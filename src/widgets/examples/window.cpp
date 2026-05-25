#include <loom/TzCloseEvent>
#include <loom/TzGuiApplication>
#include <loom/TzKeyEvent>
#include <loom/TzMouseEvent>
#include <loom/TzPaintEvent>
#include <loom/TzPainter>
#include <loom/TzScopedPointer>
#include <loom/TzTimer>
#include <loom/TzWindow>

#include <cmath>
#include <cstdint>
#include <print>

static constexpr int kFps = 60;

class ExampleWindow : public TzWindow
{
public:
    ExampleWindow()
        : TzWindow(800, 600)
    {}

protected:
    void paintEvent(TzPaintEvent *event) override
    {
        const int w = width();
        const int h = height();
        TzPainter *p = event->painter();

        const double t = static_cast<double>(m_frame++) / kFps;

        for (int y = 0; y < h; ++y) {
            for (int x = 0; x < w; ++x) {
                const double r = std::cos(static_cast<double>(x) / w + t) * 0.5 + 0.5;
                const double g = std::sin(static_cast<double>(y) / h + t) * 0.5 + 0.5;
                const double b = std::cos(static_cast<double>(x + y) / (w + h) + t) * 0.5 + 0.5;
                // TzPainter works on the shared pixel buffer owned by TzWindow —
                // fill it row by row via a single solid-colour rect per pixel.
                p->fillRect(static_cast<double>(x), static_cast<double>(y), 1.0, 1.0,
                            (0xFFu << 24)
                            | (static_cast<uint32_t>(r * 255.0) << 16)
                            | (static_cast<uint32_t>(g * 255.0) << 8)
                            |  static_cast<uint32_t>(b * 255.0));
            }
        }
    }

    void closeEvent(TzCloseEvent *event) override
    {
        TzWindow::closeEvent(event);
        tzGuiApp->quit();
    }

    void keyEvent(TzKeyEvent *event) override
    {
        if (event->key() == Key::Escape) {
            tzGuiApp->quit();
        } else if (!event->utf8().empty()) {
            std::println("Key: {}", event->utf8());
        }
    }

    void mouseEvent(TzMouseEvent *event) override
    {
        switch (event->type()) {
        case TzEvent::MouseButtonPress:
            std::println("Mouse press   ({:.0f}, {:.0f}) button={}", event->x(), event->y(),
                         (int) event->button());
            break;
        case TzEvent::MouseButtonRelease:
            std::println("Mouse release ({:.0f}, {:.0f}) button={}", event->x(), event->y(),
                         (int) event->button());
            break;
        case TzEvent::MouseScroll:
            std::println("Scroll dx={:.1f} dy={:.1f}", event->scrollDx(), event->scrollDy());
            break;
        default:
            break;
        }
    }

    void resizeEvent(TzResizeEvent *event) override
    {
        TzWindow::resizeEvent(event);
        std::println("Resized: {}x{}", event->width(), event->height());
    }

private:
    std::size_t m_frame{0};
};

int main(int argc, char *argv[])
{
    TzGuiApplication app(argc, argv);

    auto window = tzScopedPointer(new ExampleWindow);
    window->setTitle("event-loop window");
    window->show();

    return app.exec();
}
