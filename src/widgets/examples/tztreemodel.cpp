#include "tztreemodel.hpp"
#include "tztreenode.hpp"
#include <loom/tzscopedeventlistener.hpp>
#include <tzabstractitemmodel_p.hpp>

class TzTreeModelPrivate : public TzAbstractItemModelPrivate
{
public:
    TzTreeModelPrivate();

    TzTreeNodePtr root;

    struct NodeConnections
    {
        TzScopedEventListener onChildAdded;
        TzScopedEventListener onChildRemoved;
        TzScopedEventListener onDataChanged;
        TzScopedEventListener onNameChanged;
    };
    std::unordered_map<TzTreeNode *, NodeConnections> connections;
};

TzTreeModelPrivate::TzTreeModelPrivate()
    : root(TzTreeNode::createFolder(""))
{}

TzTreeModel::TzTreeModel(const TzTreeNodePtr &node)
    : TzTreeModel(*new TzTreeModelPrivate)
{
    TZ_D(TzTreeModel);
    if (d->root) {
        d->root->addChild(node);
        connectRecursively(d->root);
    }
}

TzTreeModel::TzTreeModel(const std::vector<TzTreeNodePtr> &nodes)
    : TzTreeModel(*new TzTreeModelPrivate)
{
    TZ_D(TzTreeModel);
    for (const TzTreeNodePtr &node : nodes)
        d->root->addChild(node);
    connectRecursively(d->root);
}

TzTreeModel::~TzTreeModel()
{
    TZ_D(TzTreeModel);
    if (d->root)
        disconnectRecursively(d->root);
}

TzTreeNodePtr TzTreeModel::invisibleRoot() const
{
    TZ_D(const TzTreeModel);
    return d->root;
}

TzModelIndex TzTreeModel::index(int row, int column, const TzModelIndex &parent) const
{
    TZ_D(const TzTreeModel);
    if (!hasIndex(row, column, parent))
        return TzModelIndex();

    TzTreeNodePtr parentNode = parent.isValid() ? nodeForIndex(parent) : d->root;
    if (!parentNode || row >= static_cast<int>(parentNode->childCount()))
        return TzModelIndex();

    TzTreeNodePtr childNode = parentNode->childAt(row);
    return childNode ? createIndex(row, column, childNode.get()) : TzModelIndex();
}

TzModelIndex TzTreeModel::parent(const TzModelIndex &child) const
{
    TZ_D(const TzTreeModel);
    if (!child.isValid())
        return TzModelIndex();

    TzTreeNodePtr childNode = nodeForIndex(child);
    if (!childNode)
        return TzModelIndex();

    TzTreeNodePtr parentNode = childNode->parent();
    if (!parentNode || parentNode == d->root)
        return TzModelIndex();

    TzTreeNodePtr grandparentNode = parentNode->parent();
    if (!grandparentNode)
        return TzModelIndex();

    int row = grandparentNode->indexOf(parentNode);
    return createIndex(row, 0, parentNode.get());
}

int TzTreeModel::rowCount(const TzModelIndex &parent) const
{
    TZ_D(const TzTreeModel);
    if (parent.column() > 0)
        return 0;

    TzTreeNodePtr parentNode = parent.isValid() ? nodeForIndex(parent) : d->root;
    return parentNode ? static_cast<int>(parentNode->childCount()) : 0;
}

int TzTreeModel::columnCount(const TzModelIndex &parent) const
{
    (void) parent;
    return 1;
}

bool TzTreeModel::setData(const TzModelIndex &index, const std::any &value, int role)
{
    if (!index.isValid())
        return false;

    TzTreeNodePtr node = nodeForIndex(index);
    if (!node)
        return false;

    if (index.column() == 0) {
        if (role == TzItemDataRole::EditRole) {
            if (auto *name = std::any_cast<std::string>(&value))
                return node->setName(*name);
        }
    }

    return false;
}

std::any TzTreeModel::data(const TzModelIndex &index, int role) const
{
    if (!index.isValid())
        return std::any();

    TzTreeNodePtr node = nodeForIndex(index);
    if (!node)
        return std::any();

    if (index.column() == 0) {
        if (role == TzItemDataRole::DisplayRole || role == TzItemDataRole::EditRole)
            return node->name();
    }

    return std::any();
}

TzTreeNodePtr TzTreeModel::nodeForIndex(const TzModelIndex &index) const
{
    TZ_D(const TzTreeModel);
    if (!index.isValid())
        return d->root;

    return static_cast<TzTreeNode *>(index.internalPointer())->shared_from_this();
}

TzModelIndex TzTreeModel::indexForNode(const TzTreeNodePtr &node, int column) const
{
    TZ_D(const TzTreeModel);
    if (!node || node == d->root)
        return TzModelIndex();

    TzTreeNodePtr parent = node->parent();
    if (!parent)
        return TzModelIndex();

    int row = parent->indexOf(node);
    if (row < 0)
        return TzModelIndex();

    return createIndex(row, column, node.get());
}

TzTreeModel::TzTreeModel(TzTreeModelPrivate &dd)
    : TzAbstractItemModel(dd)
{
    m_d = &dd;
}

void TzTreeModel::connectNode(const TzTreeNodePtr &node)
{
    TZ_D(TzTreeModel);
    if (!node)
        return;

    auto &conn = d->connections[node.get()];

    conn.onChildAdded = TzScopedEventListener(
        node->events().on("childAdded",
                          [this, nodeWeak = std::weak_ptr(node)](TzTreeNodePtr child, int index) {
                              auto node = nodeWeak.lock();
                              if (!node)
                                  return;

                              TzModelIndex parentIdx = indexForNode(node);
                              beginInsertRows(parentIdx, index, index);
                              connectNode(child);
                              endInsertRows();
                          }));

    conn.onChildRemoved = TzScopedEventListener(
        node->events().on("childAboutToBeRemoved",
                          [this, nodeWeak = std::weak_ptr(node)](TzTreeNodePtr child, int index) {
                              auto node = nodeWeak.lock();
                              if (!node)
                                  return;

                              TzModelIndex parentIdx = indexForNode(node);
                              beginRemoveRows(parentIdx, index, index);
                              disconnectNode(child);
                              endRemoveRows();
                          }));

    conn.onDataChanged = TzScopedEventListener(
        node->events().on("dataChanged", [this, nodeWeak = std::weak_ptr(node)]() {
            auto node = nodeWeak.lock();
            if (!node)
                return;

            TzModelIndex idx = indexForNode(node);
            if (idx.isValid()) {
                TzModelIndex lastCol = index(idx.row(), 0, parent(idx));
                dataChanged(idx, lastCol, {});
            }
        }));

    conn.onNameChanged = TzScopedEventListener(
        node->events().on("nameChanged",
                          [this, nodeWeak = std::weak_ptr(node)](const std::string &) {
                              auto node = nodeWeak.lock();
                              if (!node)
                                  return;

                              TzModelIndex idx = indexForNode(node);
                              if (idx.isValid()) {
                                  dataChanged(idx, idx, {TzItemDataRole::DisplayRole});
                              }
                          }));
}

void TzTreeModel::disconnectNode(const TzTreeNodePtr &node)
{
    TZ_D(TzTreeModel);
    if (!node)
        return;

    d->connections.erase(node.get());
}

void TzTreeModel::connectRecursively(const TzTreeNodePtr &node)
{
    if (!node)
        return;

    connectNode(node);

    for (const auto &child : node->children())
        connectRecursively(child);
}

void TzTreeModel::disconnectRecursively(const TzTreeNodePtr &node)
{
    if (!node)
        return;

    for (const auto &child : node->children())
        disconnectRecursively(child);

    disconnectNode(node);
}
