// merge_materials.cpp
//
// Demonstrates TzMergeModel: three independent TzTreeModel instances (metals,
// fabric, wood) are unified under a single virtual tree:
//
//   home
//   └── materials
//       ├── metals
//       │   ├── Iron
//       │   ├── Copper
//       │   │   ├── Electrolytic
//       │   │   └── Beryllium
//       │   └── Steel
//       │       ├── Carbon Steel
//       │       └── Stainless
//       ├── fabric
//       │   ├── Cotton
//       │   ├── Silk
//       │   └── Wool
//       │       ├── Merino
//       │       └── Cashmere
//       └── wood
//           ├── Oak
//           │   ├── White Oak
//           │   └── Red Oak
//           ├── Pine
//           └── Maple

#include <loom/TzAnchors>
#include <loom/TzGuiApplication>
#include <loom/TzKeyEvent>
#include <loom/TzMouseEvent>
#include <loom/TzPainter>
#include <loom/TzScopedEventListener>
#include <loom/TzWidget>
#include <loom/TzWindow>

#include "tzmergemodel.hpp"
#include "tztreenode.hpp"
#include "tztreemodel.hpp"

#include <any>
#include <set>
#include <string>
#include <vector>

// ---------------------------------------------------------------------------
// Colour palette (Catppuccin Mocha)
// ---------------------------------------------------------------------------
static constexpr uint32_t kBg       = 0xFF1E1E2E;
static constexpr uint32_t kSurface  = 0xFF313244;
static constexpr uint32_t kBorder   = 0xFF45475A;
static constexpr uint32_t kText     = 0xFFCDD6F4;
static constexpr uint32_t kTextDim  = 0xFF6C7086;
static constexpr uint32_t kAccent   = 0xFF89B4FA;
static constexpr uint32_t kHover    = 0xFF45475A;
static constexpr uint32_t kSelected = 0xFF585B70;

// Node-kind colours
static constexpr uint32_t kColRoot     = 0xFFF38BA8; // pink  – home
static constexpr uint32_t kColCategory = 0xFFFAB387; // peach – materials
static constexpr uint32_t kColGroup    = 0xFFA6E3A1; // green – metals/fabric/wood
static constexpr uint32_t kColFolder   = 0xFFF9E2AF; // yellow – source folders
static constexpr uint32_t kColItem     = 0xFFCDD6F4; // text  – leaf items

static constexpr double kRowH  = 26.0;
static constexpr double kCW    = 8.0;  // char width
static constexpr double kCH    = 8.0;  // char height
static constexpr double kIndent = 16.0;

// ---------------------------------------------------------------------------
// TreeViewWidget – renders TzMergeModel as an expandable tree
// ---------------------------------------------------------------------------
class TreeViewWidget : public TzWidget
{
public:
    explicit TreeViewWidget(TzMergeModel &model, TzWidget *parent = nullptr)
        : TzWidget(parent)
        , m_model(model)
    {
        rebuild();

        m_onChange = TzScopedEventListener(model.events().on("modelReset", [this]() {
            this->rebuild();
            this->update();
        }));
        m_onRowsInserted = TzScopedEventListener(model.events().on("rowsInserted",
            [this](const TzModelIndex &, int, int) { this->rebuild(); this->update(); }));
        m_onRowsRemoved = TzScopedEventListener(model.events().on("rowsRemoved",
            [this](const TzModelIndex &, int, int) { this->rebuild(); this->update(); }));
        m_onDataChanged = TzScopedEventListener(model.events().on("dataChanged",
            [this](const TzModelIndex &, const TzModelIndex &,
                   const std::vector<int> &) { this->update(); }));
    }

protected:
    void paint(TzPainter *p) override
    {
        p->fillRect(0, 0, width(), height(), kBg);

        for (int i = 0; i < static_cast<int>(m_rows.size()); ++i) {
            const Row &row = m_rows[static_cast<std::size_t>(i)];
            double ry = static_cast<double>(i) * kRowH;

            if (i == m_hoveredRow)
                p->fillRect(0, ry, width(), kRowH, kHover);
            if (i == m_selectedRow)
                p->fillRect(0, ry, width(), kRowH, kSelected);

            double tx = row.depth * kIndent + 4.0;
            double ty = ry + (kRowH - kCH) / 2.0;

            // Expand / collapse triangle (or leaf dot)
            if (row.hasChildren) {
                const char *arrow = row.expanded ? "v" : ">";
                p->drawText(tx, ty, arrow, kTextDim);
            } else {
                p->drawText(tx, ty, ".", kTextDim);
            }
            tx += kCW + 4.0;

            // Label with kind-specific colour
            uint32_t col = labelColor(row.kind);
            p->drawText(tx, ty, row.label, col);

            // Child-count hint for expandable nodes
            if (row.hasChildren && !row.expanded) {
                std::string hint = "(" + std::to_string(row.childCount) + ")";
                double hx = tx + static_cast<double>(row.label.size()) * kCW + 8.0;
                p->drawText(hx, ty, hint, kTextDim);
            }

            // Separator
            p->fillRect(0, ry + kRowH - 1.0, width(), 1.0, kBorder);
        }

        if (m_rows.empty())
            p->drawText(12.0, 12.0, "(empty)", kTextDim);
    }

    bool event(TzEvent *e) override
    {
        if (e->type() == TzEvent::MouseMove) {
            auto *me = static_cast<TzMouseEvent *>(e);
            int r = rowAt(me->y());
            if (r != m_hoveredRow) { m_hoveredRow = r; update(); }
            return true;
        }
        if (e->type() == TzEvent::MouseButtonPress) {
            auto *me = static_cast<TzMouseEvent *>(e);
            if (me->button() == MouseButton::Left) {
                int r = rowAt(me->y());
                m_selectedRow = r;
                if (r >= 0 && r < static_cast<int>(m_rows.size()))
                    toggleRow(r);
                update();
            }
            return true;
        }
        if (e->type() == TzEvent::KeyPress) {
            auto *ke = static_cast<TzKeyEvent *>(e);
            if (ke->key() == Key::Up && m_selectedRow > 0) {
                --m_selectedRow; update(); return true;
            }
            if (ke->key() == Key::Down
                && m_selectedRow < static_cast<int>(m_rows.size()) - 1) {
                ++m_selectedRow; update(); return true;
            }
            if (ke->key() == Key::Enter && m_selectedRow >= 0) {
                toggleRow(m_selectedRow); update(); return true;
            }
        }
        return TzWidget::event(e);
    }

private:
    enum class NodeKind { Root, Category, Group, Folder, Item };

    struct Row {
        TzModelIndex idx;
        std::string  label;
        int          depth{0};
        NodeKind     kind{NodeKind::Item};
        bool         hasChildren{false};
        bool         expanded{false};
        int          childCount{0};
    };

    static uint32_t labelColor(NodeKind k)
    {
        switch (k) {
        case NodeKind::Root:     return kColRoot;
        case NodeKind::Category: return kColCategory;
        case NodeKind::Group:    return kColGroup;
        case NodeKind::Folder:   return kColFolder;
        case NodeKind::Item:     return kColItem;
        }
        return kText;
    }

    // Classify a model index by depth relative to root.
    // Depth 0 = home, 1 = materials, 2 = group, 3+ = source content.
    // We derive depth from the index chain.
    static int depthOf(const TzModelIndex &idx)
    {
        int d = 0;
        TzModelIndex cur = idx;
        while (cur.isValid()) { ++d; cur = cur.model()->parent(cur); }
        return d - 1; // 0-based from home
    }

    NodeKind kindFor(const Row &row) const
    {
        if (row.depth == 0) return NodeKind::Root;
        if (row.depth == 1) return NodeKind::Category;
        if (row.depth == 2) return NodeKind::Group;
        return row.hasChildren ? NodeKind::Folder : NodeKind::Item;
    }

    // Build the flat visible-row list from the model, honouring m_expanded.
    void rebuild()
    {
        m_rows.clear();
        // Start from the root (invisible root has 1 child: home)
        TzModelIndex homeIdx = m_model.index(0, 0);
        if (homeIdx.isValid())
            appendRows(homeIdx, 0);
    }

    void appendRows(const TzModelIndex &idx, int depth)
    {
        if (!idx.isValid()) return;

        Row row;
        row.idx   = idx;
        row.depth = depth;
        auto v = m_model.data(idx, TzItemDataRole::DisplayRole);
        if (auto *s = std::any_cast<std::string>(&v))
            row.label = *s;
        row.childCount = m_model.rowCount(idx);
        row.hasChildren = row.childCount > 0;
        row.expanded = m_expanded.count(idx.internalId()) > 0;
        row.kind = kindFor(row);
        m_rows.push_back(row);

        if (row.expanded) {
            for (int r = 0; r < row.childCount; ++r)
                appendRows(m_model.index(r, 0, idx), depth + 1);
        }
    }

    void toggleRow(int i)
    {
        Row &row = m_rows[static_cast<std::size_t>(i)];
        if (!row.hasChildren) return;
        uintptr_t id = row.idx.internalId();
        if (m_expanded.count(id))
            m_expanded.erase(id);
        else
            m_expanded.insert(id);
        rebuild();
    }

    int rowAt(double y) const
    {
        if (y < 0) return -1;
        int r = static_cast<int>(y / kRowH);
        return r < static_cast<int>(m_rows.size()) ? r : -1;
    }

    TzMergeModel &m_model;
    std::vector<Row> m_rows;
    std::set<uintptr_t> m_expanded;
    int m_hoveredRow{-1};
    int m_selectedRow{-1};
    TzScopedEventListener m_onChange;
    TzScopedEventListener m_onRowsInserted;
    TzScopedEventListener m_onRowsRemoved;
    TzScopedEventListener m_onDataChanged;
};

// ---------------------------------------------------------------------------
// Status bar
// ---------------------------------------------------------------------------
class InfoBar : public TzWidget
{
public:
    explicit InfoBar(TzWidget *parent = nullptr) : TzWidget(parent)
    {
        setImplicitHeight(22.0);
    }
    void setText(const std::string &t) { m_text = t; update(); }
protected:
    void paint(TzPainter *p) override
    {
        p->fillRect(0, 0, width(), height(), kSurface);
        p->fillRect(0, 0, width(), 1.0, kBorder);
        p->drawText(8.0, (height() - kCH) / 2.0, m_text, kTextDim);
    }
private:
    std::string m_text;
};

// ---------------------------------------------------------------------------
// Root widget
// ---------------------------------------------------------------------------
class MergeBrowserRoot : public TzWidget
{
public:
    explicit MergeBrowserRoot(TzGuiApplication &app, TzMergeModel &model)
        : TzWidget(nullptr), m_app(app)
    {
        m_info = new InfoBar(this);
        m_info->setText(
            "Click or Enter to expand/collapse   |   "
            "Up/Down to navigate   |   Esc to quit");
        m_info->anchors()->setLeft(this,   TzAnchors::Left);
        m_info->anchors()->setRight(this,  TzAnchors::Right);
        m_info->anchors()->setBottom(this, TzAnchors::Bottom);

        m_tree = new TreeViewWidget(model, this);
        m_tree->anchors()->setLeft(this,  TzAnchors::Left);
        m_tree->anchors()->setRight(this, TzAnchors::Right);
        m_tree->anchors()->setTop(this,   TzAnchors::Top);
        m_tree->anchors()->setBottom(m_info, TzAnchors::Top);
        m_tree->setFocus();
    }

protected:
    void paint(TzPainter *p) override
    { p->fillRect(0, 0, width(), height(), kBg); }

    bool event(TzEvent *e) override
    {
        if (e->type() == TzEvent::KeyPress) {
            if (static_cast<TzKeyEvent *>(e)->key() == Key::Escape) {
                m_app.quit(); return true;
            }
        }
        return TzWidget::event(e);
    }

private:
    TzGuiApplication &m_app;
    TreeViewWidget   *m_tree{nullptr};
    InfoBar          *m_info{nullptr};
};

// ---------------------------------------------------------------------------
// Demo data builders
// ---------------------------------------------------------------------------
static TzTreeModel *buildMetalsModel()
{
    auto iron   = TzTreeNode::createItem("Iron");
    auto copper = TzTreeNode::createFolder("Copper");
    TzTreeNode::createItem("Electrolytic")->setParent(copper);
    TzTreeNode::createItem("Beryllium")->setParent(copper);
    auto steel  = TzTreeNode::createFolder("Steel");
    TzTreeNode::createItem("Carbon Steel")->setParent(steel);
    TzTreeNode::createItem("Stainless")->setParent(steel);
    return new TzTreeModel({iron, copper, steel});
}

static TzTreeModel *buildFabricModel()
{
    auto cotton = TzTreeNode::createItem("Cotton");
    auto silk   = TzTreeNode::createItem("Silk");
    auto wool   = TzTreeNode::createFolder("Wool");
    TzTreeNode::createItem("Merino")->setParent(wool);
    TzTreeNode::createItem("Cashmere")->setParent(wool);
    return new TzTreeModel({cotton, silk, wool});
}

static TzTreeModel *buildWoodModel()
{
    auto oak  = TzTreeNode::createFolder("Oak");
    TzTreeNode::createItem("White Oak")->setParent(oak);
    TzTreeNode::createItem("Red Oak")->setParent(oak);
    auto pine  = TzTreeNode::createItem("Pine");
    auto maple = TzTreeNode::createItem("Maple");
    return new TzTreeModel({oak, pine, maple});
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    TzGuiApplication app(argc, argv);

    // Build source models
    auto *metals = buildMetalsModel();
    auto *fabric = buildFabricModel();
    auto *wood   = buildWoodModel();

    // Build merge model
    TzMergeModel model("home", "materials");
    model.addSubModel("metals", metals);
    model.addSubModel("fabric", fabric);
    model.addSubModel("wood",   wood);

    // Window
    TzWindow window(480, 560);
    window.setTitle("Merge Materials Browser");
    window.setOnClose([&] { app.quit(); });

    auto *root = new MergeBrowserRoot(app, model);
    window.setRootWidget(root);
    window.show();

    return app.exec();
}
