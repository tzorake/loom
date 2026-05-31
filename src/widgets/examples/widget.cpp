#include <loom/TzAnchors>
#include <loom/TzCloseEvent>
#include <loom/TzGuiApplication>
#include <loom/TzKeyEvent>
#include <loom/TzMouseEvent>
#include <loom/TzPainter>
#include <loom/TzRect>
#include <loom/TzWidget>
#include <loom/TzWindow>
#include <loom/TzLogging>
#include <string>
#include <functional>

static constexpr uint32_t kColorBg = 0xFF1E1E2E;
static constexpr uint32_t kColorHeader = 0xFF313244;
static constexpr uint32_t kColorPanel = 0xFF181825;
static constexpr uint32_t kColorBorder = 0xFF45475A;
static constexpr uint32_t kColorText = 0xFFCDD6F4;
static constexpr uint32_t kColorTextDim = 0xFF6C7086;
static constexpr uint32_t kColorAccent = 0xFF89B4FA;
static constexpr uint32_t kColorButtonNorm = 0xFF313244;
static constexpr uint32_t kColorButtonHover = 0xFF45475A;
static constexpr uint32_t kColorButtonFocus = 0xFF89B4FA;
static constexpr uint32_t kColorButtonText = 0xFFCDD6F4;

class Label : public TzWidget
{
public:
    explicit Label(const std::string &text, TzWidget *parent = nullptr)
        : TzWidget(parent)
        , m_text(text)
    {
        setImplicitSize((double) (m_text.size() * 8), 16.0);
    }

    void setText(const std::string &text)
    {
        m_text = text;
        setImplicitSize((double) (m_text.size() * 8), 16.0);
        update();
    }

    const std::string &text() const { return m_text; }

protected:
    void paint(TzPainter *p) override
    {
        double ty = (height() - 8.0) / 2.0;
        p->drawText(0, ty, m_text, kColorText);
    }

private:
    std::string m_text;
};

class Button : public TzWidget
{
public:
    using ClickCallback = std::function<void()>;

    explicit Button(const std::string &label, TzWidget *parent = nullptr)
        : TzWidget(parent)
        , m_label(label)
    {
        setImplicitSize(120.0, 32.0);
    }

    void setOnClick(ClickCallback cb) { m_onClick = std::move(cb); }

protected:
    void paint(TzPainter *p) override
    {
        uint32_t bg = hasFocus()  ? kColorButtonFocus
                      : m_hovered ? kColorButtonHover
                                  : kColorButtonNorm;
        uint32_t textColor = hasFocus() ? kColorPanel : kColorButtonText;

        p->fillRect(0.0, 0.0, width(), height(), bg);
        p->drawRect({0.0, 0.0, width(), height()}, kColorBorder);

        double tx = (width() - (double) (m_label.size() * 8)) / 2.0;
        double ty = (height() - 8.0) / 2.0;
        p->drawText(tx, ty, m_label, textColor);
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

    void geometryChanged(const TzRect &, const TzRect &) override { update(); }

private:
    std::string m_label;
    ClickCallback m_onClick;
    bool m_hovered{false};
};

class CounterPanel : public TzWidget
{
public:
    explicit CounterPanel(TzWidget *parent = nullptr)
        : TzWidget(parent)
    {
        // Header bar
        m_header = new TzWidget(this);
        m_header->setImplicitHeight(40.0);
        m_header->anchors()->setLeft(this, TzAnchors::Left);
        m_header->anchors()->setRight(this, TzAnchors::Right);
        m_header->anchors()->setTop(this, TzAnchors::Top);

        m_headerLabel = new Label("Counter Demo", m_header);
        m_headerLabel->anchors()->setLeft(m_header, TzAnchors::Left, 8.0);
        m_headerLabel->anchors()->setVCenter(m_header, TzAnchors::VCenter);

        // Body area (below header)
        m_body = new TzWidget(this);
        m_body->anchors()->setLeft(this, TzAnchors::Left);
        m_body->anchors()->setRight(this, TzAnchors::Right);
        m_body->anchors()->setTop(m_header, TzAnchors::Bottom);
        m_body->anchors()->setBottom(this, TzAnchors::Bottom);

        // Count display
        m_countLabel = new Label("0", m_body);
        m_countLabel->anchors()->centerIn(m_body);

        // Button row container — 44 px tall, anchored to bottom of body
        m_buttonRow = new TzWidget(m_body);
        m_buttonRow->setImplicitHeight(44.0);
        m_buttonRow->anchors()->setLeft(m_body, TzAnchors::Left, 16.0);
        m_buttonRow->anchors()->setRight(m_body, TzAnchors::Right, 16.0);
        m_buttonRow->anchors()->setBottom(m_body, TzAnchors::Bottom, 12.0);

        // Decrement button — left half of row
        m_decBtn = new Button("-1", m_buttonRow);
        m_decBtn->anchors()->setLeft(m_buttonRow, TzAnchors::Left);
        m_decBtn->anchors()->setTop(m_buttonRow, TzAnchors::Top);
        m_decBtn->anchors()->setBottom(m_buttonRow, TzAnchors::Bottom);
        m_decBtn->anchors()->setRight(m_buttonRow, TzAnchors::HCenter, 4.0);
        m_decBtn->setOnClick([this] { decrement(); });

        // Increment button — right half of row
        m_incBtn = new Button("+1", m_buttonRow);
        m_incBtn->anchors()->setLeft(m_buttonRow, TzAnchors::HCenter, 4.0);
        m_incBtn->anchors()->setTop(m_buttonRow, TzAnchors::Top);
        m_incBtn->anchors()->setBottom(m_buttonRow, TzAnchors::Bottom);
        m_incBtn->anchors()->setRight(m_buttonRow, TzAnchors::Right);
        m_incBtn->setOnClick([this] { increment(); });
    }

    int count() const { return m_count; }

protected:
    void paint(TzPainter *p) override
    {
        p->fillRect(0.0, 0.0, width(), height(), kColorPanel);
        p->drawRect({0.0, 0.0, width(), height()}, kColorBorder);
    }

private:
    void increment()
    {
        ++m_count;
        m_countLabel->setText(std::to_string(m_count));
        tzInfo("count = {}", m_count);
    }

    void decrement()
    {
        --m_count;
        m_countLabel->setText(std::to_string(m_count));
        tzInfo("count = {}", m_count);
    }

    int m_count{0};
    TzWidget *m_header{nullptr};
    Label *m_headerLabel{nullptr};
    TzWidget *m_body{nullptr};
    Label *m_countLabel{nullptr};
    TzWidget *m_buttonRow{nullptr};
    Button *m_decBtn{nullptr};
    Button *m_incBtn{nullptr};
};

class HeaderBar : public TzWidget
{
public:
    explicit HeaderBar(TzWidget *parent = nullptr)
        : TzWidget(parent)
    {
        m_title = new Label("event-loop widget demo", this);
        m_title->anchors()->setLeft(this, TzAnchors::Left, 12.0);
        m_title->anchors()->setVCenter(this, TzAnchors::VCenter);

        m_hint = new Label("Press Esc to quit", this);
        m_hint->anchors()->setRight(this, TzAnchors::Right, 12.0);
        m_hint->anchors()->setVCenter(this, TzAnchors::VCenter);
    }

protected:
    void paint(TzPainter *p) override
    {
        p->fillRect(0.0, 0.0, width(), height(), kColorHeader);
        // Bottom border line
        p->fillRect(0.0, height() - 1.0, width(), 1.0, kColorAccent);
    }

private:
    Label *m_title{nullptr};
    Label *m_hint{nullptr};
};

class StatusBar : public TzWidget
{
public:
    explicit StatusBar(TzWidget *parent = nullptr)
        : TzWidget(parent)
    {
        m_label = new Label("Ready", this);
        m_label->anchors()->setLeft(this, TzAnchors::Left, 8.0);
        m_label->anchors()->setVCenter(this, TzAnchors::VCenter);
    }

    void setMessage(const std::string &msg) { m_label->setText(msg); }

protected:
    void paint(TzPainter *p) override
    {
        p->fillRect(0.0, 0.0, width(), height(), kColorHeader);
        // Top border
        p->fillRect(0.0, 0.0, width(), 1.0, kColorBorder);
    }

private:
    Label *m_label{nullptr};
};

class RootWidget : public TzWidget
{
public:
    RootWidget()
        : TzWidget(nullptr)
    {
        // Header: 36 px at top
        m_header = new HeaderBar(this);
        m_header->setImplicitHeight(36.0);
        m_header->anchors()->setLeft(this, TzAnchors::Left);
        m_header->anchors()->setRight(this, TzAnchors::Right);
        m_header->anchors()->setTop(this, TzAnchors::Top);

        // Status bar: 24 px at bottom
        m_status = new StatusBar(this);
        m_status->setImplicitHeight(24.0);
        m_status->anchors()->setLeft(this, TzAnchors::Left);
        m_status->anchors()->setRight(this, TzAnchors::Right);
        m_status->anchors()->setBottom(this, TzAnchors::Bottom);

        // Content area: between header and status bar
        m_content = new TzWidget(this);
        m_content->anchors()->setLeft(this, TzAnchors::Left, 16.0);
        m_content->anchors()->setRight(this, TzAnchors::Right, 16.0);
        m_content->anchors()->setTop(m_header, TzAnchors::Bottom, 16.0);
        m_content->anchors()->setBottom(m_status, TzAnchors::Top, 16.0);

        // Two counter panels side by side in the content area
        m_leftPanel = new CounterPanel(m_content);
        m_leftPanel->anchors()->setLeft(m_content, TzAnchors::Left);
        m_leftPanel->anchors()->setTop(m_content, TzAnchors::Top);
        m_leftPanel->anchors()->setBottom(m_content, TzAnchors::Bottom);
        m_leftPanel->anchors()->setRight(m_content, TzAnchors::HCenter, 8.0);

        m_rightPanel = new CounterPanel(m_content);
        m_rightPanel->anchors()->setLeft(m_content, TzAnchors::HCenter, 8.0);
        m_rightPanel->anchors()->setTop(m_content, TzAnchors::Top);
        m_rightPanel->anchors()->setBottom(m_content, TzAnchors::Bottom);
        m_rightPanel->anchors()->setRight(m_content, TzAnchors::Right);
    }

    void setStatus(const std::string &msg) { m_status->setMessage(msg); }

protected:
    void paint(TzPainter *p) override { p->fillRect(0.0, 0.0, width(), height(), kColorBg); }

    bool event(TzEvent *e) override
    {
        if (e->type() == TzEvent::KeyPress) {
            auto *ke = static_cast<TzKeyEvent *>(e);
            if (ke->key() == Key::Escape) {
                tzGuiApp->quit();
                return true;
            }
            if (!ke->utf8().empty())
                setStatus("Key: " + ke->utf8());
            return true;
        }
        return TzWidget::event(e);
    }

private:
    HeaderBar *m_header{nullptr};
    StatusBar *m_status{nullptr};
    TzWidget *m_content{nullptr};
    CounterPanel *m_leftPanel{nullptr};
    CounterPanel *m_rightPanel{nullptr};
};

class AppWindow : public TzWindow
{
public:
    AppWindow() : TzWindow(800, 600) {}

protected:
    void closeEvent(TzCloseEvent *event) override
    {
        TzWindow::closeEvent(event);
        tzGuiApp->quit();
    }
};

int loom_main(int argc, char *argv[])
{
    // Heap-allocate for web (WASM) compat: exec() returns immediately on web
    // so the event loop is driven by requestAnimationFrame after loom_main()
    // returns.  Stack-allocated objects would be destroyed at that point.
    // On native, exec() blocks, so heap vs stack makes no difference for
    // correctness (the small leak at process exit is benign).
    auto *app    = new TzGuiApplication(argc, argv);
    auto *window = new AppWindow();
    window->setTitle("event-loop widget demo");

    auto *root = new RootWidget();
    window->setRootWidget(root);

    window->show();

    return app->exec();
}
