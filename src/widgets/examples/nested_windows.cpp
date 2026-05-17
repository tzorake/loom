#include <loom/TzAnchors>
#include <loom/TzGuiApplication>
#include <loom/TzKeyEvent>
#include <loom/TzMouseEvent>
#include <loom/TzPainter>
#include <loom/TzRect>
#include <loom/TzScene>
#include <loom/TzWidget>
#include <loom/TzWindow>

#include <functional>
#include <string>

// ── Palette ───────────────────────────────────────────────────────────────

static constexpr uint32_t kBg = 0xFF1E1E2E;
static constexpr uint32_t kPanel = 0xFF313244;
static constexpr uint32_t kBorder = 0xFF45475A;
static constexpr uint32_t kText = 0xFFCDD6F4;
static constexpr uint32_t kAccent = 0xFF89B4FA;
static constexpr uint32_t kBtnNorm = 0xFF313244;
static constexpr uint32_t kBtnHover = 0xFF45475A;
static constexpr uint32_t kBtnFocus = 0xFF89B4FA;
static constexpr uint32_t kBtnText = 0xFFCDD6F4;
static constexpr uint32_t kBtnFocTxt = 0xFF1E1E2E;

// ── Label ─────────────────────────────────────────────────────────────────

class Label : public TzWidget
{
public:
    explicit Label(const std::string &text, TzWidget *parent = nullptr)
        : TzWidget(parent)
        , m_text(text)
    {
        updateImplicit();
    }

    void setText(const std::string &text)
    {
        m_text = text;
        updateImplicit();
        update();
    }

protected:
    void paint(TzPainter *p) override
    {
        double ty = (height() - 8.0) / 2.0;
        p->drawText(0.0, ty, m_text, kText);
    }

private:
    void updateImplicit() { setImplicitSize(double(m_text.size() * 8), 16.0); }

    std::string m_text;
};

// ── Button ────────────────────────────────────────────────────────────────

class Button : public TzWidget
{
public:
    using Callback = std::function<void()>;

    explicit Button(const std::string &label, TzWidget *parent = nullptr)
        : TzWidget(parent)
        , m_label(label)
    {
        setImplicitSize(140.0, 32.0);
    }

    void setOnClick(Callback cb) { m_onClick = std::move(cb); }

protected:
    void paint(TzPainter *p) override
    {
        uint32_t bg = hasFocus() ? kBtnFocus : m_hovered ? kBtnHover : kBtnNorm;
        uint32_t fg = hasFocus() ? kBtnFocTxt : kBtnText;
        p->fillRect(0.0, 0.0, width(), height(), bg);
        p->drawRect({0.0, 0.0, width(), height()}, kBorder);
        double tx = (width() - double(m_label.size() * 8)) / 2.0;
        double ty = (height() - 8.0) / 2.0;
        p->drawText(tx, ty, m_label, fg);
    }

    bool event(TzEvent *e) override
    {
        if (e->type() == TzEvent::MouseButtonPress) {
            if (m_onClick)
                m_onClick();
            return true;
        }
        if (e->type() == TzEvent::MouseMove) {
            if (!m_hovered) {
                m_hovered = true;
                update();
            }
            return true;
        }
        return TzWidget::event(e);
    }

private:
    std::string m_label;
    Callback m_onClick;
    bool m_hovered{false};
};

// ── Dialog window root widget ─────────────────────────────────────────────

class DialogRoot : public TzWidget
{
public:
    explicit DialogRoot(TzWindow *owner)
        : TzWidget(nullptr)
        , m_owner(owner)
    {
        m_title = new Label("This is a child window!", this);
        m_title->anchors()->centerIn(this);

        m_closeBtn = new Button("Close", this);
        m_closeBtn->anchors()->setHCenter(this, TzAnchors::HCenter);
        m_closeBtn->anchors()->setBottom(this, TzAnchors::Bottom, 16.0);
        m_closeBtn->setOnClick([this] { m_owner->close(); });
    }

protected:
    void paint(TzPainter *p) override
    {
        p->fillRect(0.0, 0.0, width(), height(), kPanel);
        p->drawRect({0.0, 0.0, width(), height()}, kAccent);
    }

    bool event(TzEvent *e) override
    {
        if (e->type() == TzEvent::KeyPress) {
            auto *ke = static_cast<TzKeyEvent *>(e);
            if (ke->key() == Key::Escape) {
                m_owner->close();
                return true;
            }
        }
        return TzWidget::event(e);
    }

private:
    TzWindow *m_owner;
    Label *m_title{nullptr};
    Button *m_closeBtn{nullptr};
};

class MainRoot : public TzWidget
{
public:
    explicit MainRoot(TzGuiApplication &app, TzWindow *owner)
        : TzWidget(nullptr)
        , m_app(app)
        , m_owner(owner)
    {
        m_hint = new Label("Press Esc to quit", this);
        m_hint->anchors()->setTop(this, TzAnchors::Top, 16.0);
        m_hint->anchors()->setHCenter(this, TzAnchors::HCenter);

        m_openBtn = new Button("Open child window", this);
        m_openBtn->anchors()->setHCenter(this, TzAnchors::HCenter);
        m_openBtn->anchors()->setVCenter(this, TzAnchors::VCenter, -24.0);
        m_openBtn->setOnClick([this] { openDialog(); });

        m_quitBtn = new Button("Quit", this);
        m_quitBtn->anchors()->setHCenter(this, TzAnchors::HCenter);
        m_quitBtn->anchors()->setVCenter(this, TzAnchors::VCenter, 24.0);
        m_quitBtn->setOnClick([this] { m_app.quit(); });
    }

protected:
    void paint(TzPainter *p) override { p->fillRect(0.0, 0.0, width(), height(), kBg); }

    bool event(TzEvent *e) override
    {
        if (e->type() == TzEvent::KeyPress) {
            auto *ke = static_cast<TzKeyEvent *>(e);
            if (ke->key() == Key::Escape) {
                m_app.quit();
                return true;
            }
        }
        return TzWidget::event(e);
    }

private:
    void openDialog()
    {
        // Child window — owned by m_owner (TzObject parent), auto-destroyed with it
        auto *dialog = new TzWindow(400, 240, m_owner);
        dialog->setTitle("Child window");
        auto *root = new DialogRoot(dialog);
        dialog->setRootWidget(root);
        dialog->show();
    }

    TzGuiApplication &m_app;
    TzWindow *m_owner;
    Label *m_hint{nullptr};
    Button *m_openBtn{nullptr};
    Button *m_quitBtn{nullptr};
};

int main(int argc, char *argv[])
{
    TzGuiApplication app(argc, argv);

    TzWindow mainWindow(800, 600);
    mainWindow.setTitle("Nested windows demo");
    mainWindow.setOnClose([&] { app.quit(); });

    auto *root = new MainRoot(app, &mainWindow);
    mainWindow.setRootWidget(root);

    mainWindow.show();

    return app.exec();
}
