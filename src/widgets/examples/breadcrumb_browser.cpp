#include <loom/TzGuiApplication>
#include <loom/TzWindow>
#include <loom/TzWidget>
#include <loom/TzAnchors>
#include <loom/TzPainter>
#include <loom/TzKeyEvent>
#include <loom/TzMouseEvent>
#include <loom/TzRect>
#include <loom/TzScopedEventListener>

#include "tzbreadcrumbmodel.hpp"
#include "tztreenode.hpp"

#include <string>
#include <vector>
#include <algorithm>

static constexpr uint32_t kBg = 0xFF1E1E2E;
static constexpr uint32_t kSurface = 0xFF313244;
static constexpr uint32_t kBorder = 0xFF45475A;
static constexpr uint32_t kText = 0xFFCDD6F4;
static constexpr uint32_t kTextDim = 0xFF6C7086;
static constexpr uint32_t kAccent = 0xFF89B4FA;
static constexpr uint32_t kHover = 0xFF45475A;
static constexpr uint32_t kSelected = 0xFF585B70;
static constexpr uint32_t kFolderIcon = 0xFFF9E2AF;
static constexpr uint32_t kFileIcon = 0xFFA6E3A1;

static constexpr double kRowHeight = 28.0;
static constexpr double kBarHeight = 36.0;
static constexpr double kCharWidth = 8.0;
static constexpr double kCharHeight = 8.0;

class BreadcrumbBar : public TzWidget
{
public:
    explicit BreadcrumbBar(TzBreadcrumbModel& model, TzWidget* parent = nullptr)
        : TzWidget(parent)
        , m_model(model)
    {
        setImplicitHeight(kBarHeight);
        rebuild();

        m_onChanged = m_model.events().on("currentNodeChanged",
            [this](const TzTreeNodePtr&, const TzTreeNodePtr&) {
                rebuild();
                update();
            });
    }

protected:
    void paint(TzPainter* p) override
    {
        p->fillRect(0.0, 0.0, width(), height(), kSurface);
        p->fillRect(0.0, height() - 1.0, width(), 1.0, kAccent);

        double x = 12.0;
        const double ty = (height() - kCharHeight) / 2.0;

        for (int i = 0; i < static_cast<int>(m_segments.size()); ++i) {
            const auto& seg = m_segments[i];
            bool last = (i == static_cast<int>(m_segments.size()) - 1);
            uint32_t col = last ? kText : kAccent;

            if (!last && m_hoveredSegment == i)
                col = 0xFFB4D0FF;

            p->drawText(x, ty, seg.label, col);
            x += seg.width;

            if (!last) {
                p->drawText(x, ty, " › ", kTextDim);
                x += 3.0 * kCharWidth;
            }
        }
    }

    bool event(TzEvent* e) override
    {
        if (e->type() == TzEvent::MouseButtonPress) {
            auto* me = static_cast<TzMouseEvent*>(e);
            int idx = segmentAt(me->x());
            if (idx >= 0 && idx < static_cast<int>(m_segments.size()) - 1)
                m_model.navigateTo(m_segments[static_cast<std::size_t>(idx)].node);
            return true;
        }
        if (e->type() == TzEvent::MouseMove) {
            auto* me = static_cast<TzMouseEvent*>(e);
            int idx = segmentAt(me->x());
            if (idx != m_hoveredSegment) {
                m_hoveredSegment = idx;
                update();
            }
            return true;
        }
        return TzWidget::event(e);
    }

private:
    struct Segment {
        std::string label;
        double width;
        TzTreeNodePtr node;
        double x;
    };

    void rebuild()
    {
        m_segments.clear();
        double x = 12.0;
        for (auto& node : m_model.path()) {
            Segment s;
            s.label = node->name().empty() ? "/" : node->name();
            s.width = static_cast<double>(s.label.size()) * kCharWidth;
            s.node = node;
            s.x = x;
            m_segments.push_back(s);
            x += s.width + 3.0 * kCharWidth;
        }
    }

    int segmentAt(double mx) const
    {
        for (int i = 0; i < static_cast<int>(m_segments.size()); ++i) {
            const auto& s = m_segments[static_cast<std::size_t>(i)];
            if (mx >= s.x && mx < s.x + s.width)
                return i;
        }
        return -1;
    }

    TzBreadcrumbModel& m_model;
    std::vector<Segment> m_segments;
    int m_hoveredSegment{ -1 };
    TzScopedEventListener m_onChanged;
};

class FileListPanel : public TzWidget
{
public:
    explicit FileListPanel(TzBreadcrumbModel& model, TzWidget* parent = nullptr)
        : TzWidget(parent)
        , m_model(model)
    {
        reload();

        m_onChanged = m_model.events().on("currentNodeChanged",
            [this](const TzTreeNodePtr&, const TzTreeNodePtr&) {
                reload();
                update();
            });
    }

protected:
    void paint(TzPainter* p) override
    {
        p->fillRect(0.0, 0.0, width(), height(), kBg);

        if (m_rows.empty()) {
            const double tx = 16.0;
            const double ty = 16.0;
            p->drawText(tx, ty, "(empty)", kTextDim);
            return;
        }

        for (int i = 0; i < static_cast<int>(m_rows.size()); ++i) {
            const auto& row = m_rows[static_cast<std::size_t>(i)];
            const double ry = static_cast<double>(i) * kRowHeight;

            if (i == m_hoveredRow)
                p->fillRect(0.0, ry, width(), kRowHeight, kHover);
            if (i == m_selectedRow)
                p->fillRect(0.0, ry, width(), kRowHeight, kSelected);

            char icon = row.isFolder ? 'd' : 'f';
            uint32_t iconCol = row.isFolder ? kFolderIcon : kFileIcon;
            const double ty = ry + (kRowHeight - kCharHeight) / 2.0;
            p->drawText(8.0, ty, std::string(1, icon), iconCol);

            uint32_t nameCol = row.isFolder ? kText : kTextDim;
            p->drawText(28.0, ty, row.name, nameCol);

            if (row.isFolder && row.childCount > 0) {
                std::string hint = std::to_string(row.childCount) + " item"
                                 + (row.childCount != 1 ? "s" : "");
                double hx = width() - static_cast<double>(hint.size()) * kCharWidth - 12.0;
                p->drawText(hx, ty, hint, kTextDim);
            }

            p->fillRect(0.0, ry + kRowHeight - 1.0, width(), 1.0, kBorder);
        }
    }

    bool event(TzEvent* e) override
    {
        if (e->type() == TzEvent::MouseMove) {
            auto* me = static_cast<TzMouseEvent*>(e);
            int row = rowAt(me->y());
            if (row != m_hoveredRow) {
                m_hoveredRow = row;
                update();
            }
            return true;
        }
        if (e->type() == TzEvent::MouseButtonPress) {
            auto* me = static_cast<TzMouseEvent*>(e);
            if (me->button() == MouseButton::Left) {
                int row = rowAt(me->y());
                m_selectedRow = row;
                update();
                if (row >= 0 && row < static_cast<int>(m_rows.size())) {
                    const auto& r = m_rows[static_cast<std::size_t>(row)];
                    if (r.isFolder)
                        m_model.navigateTo(r.node);
                }
                return true;
            }
        }
        if (e->type() == TzEvent::KeyPress) {
            auto* ke = static_cast<TzKeyEvent*>(e);
            if (ke->key() == Key::Up) {
                if (m_selectedRow > 0) { --m_selectedRow; update(); }
                return true;
            }
            if (ke->key() == Key::Down) {
                if (m_selectedRow < static_cast<int>(m_rows.size()) - 1) {
                    ++m_selectedRow; update();
                }
                return true;
            }
            if (ke->key() == Key::Enter) {
                if (m_selectedRow >= 0 && m_selectedRow < static_cast<int>(m_rows.size())) {
                    const auto& r = m_rows[static_cast<std::size_t>(m_selectedRow)];
                    if (r.isFolder)
                        m_model.navigateTo(r.node);
                }
                return true;
            }
            if (ke->key() == Key::Backspace) {
                m_model.navigateUp();
                return true;
            }
        }
        return TzWidget::event(e);
    }

private:
    struct Row {
        std::string name;
        bool isFolder;
        int childCount;
        TzTreeNodePtr node;
    };

    void reload()
    {
        m_rows.clear();
        m_selectedRow = -1;
        m_hoveredRow = -1;

        auto current = m_model.currentNode();
        if (!current)
            return;

        for (const auto& child : current->children()) {
            Row r;
            r.name = child->name();
            r.isFolder = child->isFolder();
            r.childCount = child->childCount();
            r.node = child;
            m_rows.push_back(r);
        }

        std::stable_sort(m_rows.begin(), m_rows.end(), [](const Row& a, const Row& b) {
            if (a.isFolder != b.isFolder) return a.isFolder > b.isFolder;
            return a.name < b.name;
        });
    }

    int rowAt(double y) const
    {
        if (y < 0.0) return -1;
        int r = static_cast<int>(y / kRowHeight);
        return (r < static_cast<int>(m_rows.size())) ? r : -1;
    }

    TzBreadcrumbModel& m_model;
    std::vector<Row> m_rows;
    int m_hoveredRow{ -1 };
    int m_selectedRow{ -1 };
    TzScopedEventListener m_onChanged;
};

class StatusBar : public TzWidget
{
public:
    explicit StatusBar(TzBreadcrumbModel& model, TzWidget* parent = nullptr)
        : TzWidget(parent)
        , m_model(model)
    {
        setImplicitHeight(24.0);
        refresh();

        m_onChanged = m_model.events().on("currentNodeChanged",
            [this](const TzTreeNodePtr&, const TzTreeNodePtr&) {
                refresh();
                update();
            });
    }

protected:
    void paint(TzPainter* p) override
    {
        p->fillRect(0.0, 0.0, width(), height(), kSurface);
        p->fillRect(0.0, 0.0, width(), 1.0, kBorder);
        const double ty = (height() - kCharHeight) / 2.0;
        p->drawText(8.0, ty, m_text, kTextDim);
    }

private:
    void refresh()
    {
        auto node = m_model.currentNode();
        if (!node) { m_text = ""; return; }
        int n = node->childCount();
        m_text = std::to_string(n) + " item" + (n != 1 ? "s" : "")
               + "    Backspace = up   Enter / click = open";
    }

    TzBreadcrumbModel& m_model;
    std::string m_text;
    TzScopedEventListener m_onChanged;
};

class BreadcrumbBrowserRoot : public TzWidget
{
public:
    explicit BreadcrumbBrowserRoot(TzGuiApplication& app,
                                    TzBreadcrumbModel& model)
        : TzWidget(nullptr)
        , m_app(app)
    {
        m_bar = new BreadcrumbBar(model, this);
        m_bar->anchors()->setLeft (this, TzAnchors::Left);
        m_bar->anchors()->setRight(this, TzAnchors::Right);
        m_bar->anchors()->setTop  (this, TzAnchors::Top);

        m_status = new StatusBar(model, this);
        m_status->anchors()->setLeft  (this, TzAnchors::Left);
        m_status->anchors()->setRight (this, TzAnchors::Right);
        m_status->anchors()->setBottom(this, TzAnchors::Bottom);

        m_list = new FileListPanel(model, this);
        m_list->anchors()->setLeft  (this,     TzAnchors::Left);
        m_list->anchors()->setRight (this,     TzAnchors::Right);
        m_list->anchors()->setTop   (m_bar,    TzAnchors::Bottom);
        m_list->anchors()->setBottom(m_status, TzAnchors::Top);

        m_list->setFocus();
    }

protected:
    void paint(TzPainter* p) override
    {
        p->fillRect(0.0, 0.0, width(), height(), kBg);
    }

    bool event(TzEvent* e) override
    {
        if (e->type() == TzEvent::KeyPress) {
            auto* ke = static_cast<TzKeyEvent*>(e);
            if (ke->key() == Key::Escape) {
                m_app.quit();
                return true;
            }
        }
        return TzWidget::event(e);
    }

private:
    TzGuiApplication& m_app;
    BreadcrumbBar* m_bar{ nullptr };
    FileListPanel* m_list{ nullptr };
    StatusBar* m_status{ nullptr };
};

static TzTreeNodePtr buildDemoTree()
{
    auto root = TzTreeNode::createFolder("/");

    auto src  = TzTreeNode::createFolder("src");
    auto core = TzTreeNode::createFolder("core");
    core->addChild(TzTreeNode::createItem("tzeventloop.cpp"));
    core->addChild(TzTreeNode::createItem("tzpainter.cpp"));
    auto widgets = TzTreeNode::createFolder("widgets");
    widgets->addChild(TzTreeNode::createItem("tzwidget.cpp"));
    widgets->addChild(TzTreeNode::createItem("tzanchors.cpp"));
    auto models = TzTreeNode::createFolder("models");
    models->addChild(TzTreeNode::createItem("tzabstractitemmodel.cpp"));
    src->addChild(core);
    src->addChild(widgets);
    src->addChild(models);

    auto docs = TzTreeNode::createFolder("docs");
    docs->addChild(TzTreeNode::createItem("getting_started.md"));
    docs->addChild(TzTreeNode::createItem("api_reference.md"));

    root->addChild(src);
    root->addChild(docs);
    root->addChild(TzTreeNode::createItem("CMakeLists.txt"));

    return root;
}

int main(int argc, char* argv[])
{
    TzGuiApplication app(argc, argv);

    auto root = buildDemoTree();
    TzBreadcrumbModel model(root);

    TzWindow window(640, 480);
    window.setTitle("Breadcrumb Browser");
    window.setOnClose([&] { app.quit(); });

    auto* browser = new BreadcrumbBrowserRoot(app, model);
    window.setRootWidget(browser);
    window.show();

    return app.exec();
}
