#include <loom/TzCloseEvent>
#include <loom/TzGuiApplication>
#include <loom/TzKeyEvent>
#include <loom/TzMouseEvent>
#include <loom/TzPainter>
#include <loom/TzPoint>
#include <loom/TzRect>
#include <loom/TzTimer>
#include <loom/TzWidget>
#include <loom/TzWindow>
#include <cmath>
#include <cstdint>
#include <vector>

struct Ball {
    double x, y;
    double vx, vy;
    double radius;
    uint32_t color;
};

static uint32_t s_rng = 0xDEADBEEF;
static uint32_t rng_next()
{
    s_rng = s_rng * 1664525u + 1013904223u;
    return s_rng;
}
static double rng_f() { return double(rng_next() & 0xFFFF) / 65535.0; }
static double rng_range(double lo, double hi) { return lo + rng_f() * (hi - lo); }

class BallsWidget : public TzWidget
{
public:
    explicit BallsWidget(TzWidget *parent = nullptr)
        : TzWidget(parent)
    {
        spawnBall(400.0, 300.0);
    }

protected:
    void paint(TzPainter *p) override
    {
        p->fillRect(0.0, 0.0, width(), height(), 0xFF000000);

        for (auto &b : m_balls)
            p->drawCircle({b.x, b.y}, b.radius, b.color);
    }

    bool event(TzEvent *e) override
    {
        if (e->type() == TzEvent::MouseButtonPress) {
            auto *me = static_cast<TzMouseEvent *>(e);
            if (me->button() == MouseButton::Left) {
                spawnBall(me->x(), me->y());
                return true;
            }
        }
        return TzWidget::event(e);
    }

    void geometryChanged(const TzRect &, const TzRect &) override
    {
        update();
    }

public:
    void step()
    {
        const double W = width();
        const double H = height();
        if (W < 1.0 || H < 1.0)
            return;

        for (auto &a : m_balls) {
            a.x += a.vx;
            a.y += a.vy;

            if (a.x - a.radius < 0.0) { a.x = a.radius; a.vx = std::abs(a.vx); }
            if (a.x + a.radius > W) { a.x = W - a.radius; a.vx = -std::abs(a.vx); }
            if (a.y - a.radius < 0.0) { a.y = a.radius; a.vy = std::abs(a.vy); }
            if (a.y + a.radius > H) { a.y = H - a.radius; a.vy = -std::abs(a.vy); }
        }

        for (int i = 0; i < (int)m_balls.size(); ++i) {
            for (int j = i + 1; j < (int)m_balls.size(); ++j) {
                Ball &a = m_balls[i];
                Ball &b = m_balls[j];
                const double minDist = a.radius + b.radius;
                const double dx = b.x - a.x;
                const double dy = b.y - a.y;
                const double dist2 = dx * dx + dy * dy;
                if (dist2 >= minDist * minDist || dist2 < 1e-6)
                    continue;

                const double dist = std::sqrt(dist2);
                const double nx = dx / dist;
                const double ny = dy / dist;

                const double overlap = (minDist - dist) * 0.5;
                a.x -= nx * overlap;
                a.y -= ny * overlap;
                b.x += nx * overlap;
                b.y += ny * overlap;

                const double relVn = (a.vx - b.vx) * nx + (a.vy - b.vy) * ny;
                if (relVn > 0.0) {
                    a.vx -= relVn * nx;
                    a.vy -= relVn * ny;
                    b.vx += relVn * nx;
                    b.vy += relVn * ny;
                }
            }
        }

        update();
    }

private:
    void spawnBall(double cx, double cy)
    {
        const double r  = rng_range(10.0, 40.0);
        const double spd = rng_range(2.0, 6.0);
        const double ang = rng_f() * 6.2831853;

        uint32_t col;
        do {
            col = 0xFF000000u
                | (uint32_t(rng_next() & 0xFF) << 16)
                | (uint32_t(rng_next() & 0xFF) << 8)
                |  uint32_t(rng_next() & 0xFF);
        } while (((col >> 16 & 0xFF) + (col >> 8 & 0xFF) + (col & 0xFF)) < 180);

        const double W = width()  > 0.0 ? width()  : 800.0;
        const double H = height() > 0.0 ? height() : 600.0;
        const double x = std::max(r, std::min(cx, W - r));
        const double y = std::max(r, std::min(cy, H - r));

        m_balls.push_back({ x, y, spd * std::cos(ang), spd * std::sin(ang), r, col });
    }

    std::vector<Ball> m_balls;
};

class AppWindow : public TzWindow
{
public:
    AppWindow(int w, int h, TzGuiApplication &app)
        : TzWindow(w, h)
        , m_app(app)
    {}

protected:
    void closeEvent(TzCloseEvent *e) override { TzWindow::closeEvent(e); m_app.quit(); }

private:
    TzGuiApplication &m_app;
};

int loom_main(int argc, char *argv[])
{
    auto *app = new TzGuiApplication(argc, argv);
    auto *window = new AppWindow(800, 600, *app);
    window->setTitle("Bouncing Balls — click to spawn");

    auto *balls = new BallsWidget();
    window->setRootWidget(balls);
    window->show();

    TzTimer::repeat(std::chrono::milliseconds(16), [balls] { balls->step(); });

    return app->exec();
}
