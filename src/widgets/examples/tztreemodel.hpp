#pragma once
#include "tztreenode.hpp"
#include <loom/tzabstractitemmodel.hpp>
#include <tzabstractitemmodel_p.hpp>
#include <loom/tzclasshelpermacros.hpp>
#include <loom/tzscopedeventlistener.hpp>

class TzTreeModelPrivate;

class TzTreeModel : public TzAbstractItemModel
{
    TZ_DECLARE_PRIVATE_D(d_ptr, TzTreeModel)
public:
    TzTreeModel(const TzTreeNodePtr &node);
    TzTreeModel(const std::vector<TzTreeNodePtr> &nodes);
    virtual ~TzTreeModel() override;

    TzTreeNodePtr invisibleRoot() const;

    virtual TzModelIndex index(int row, int column,
                               const TzModelIndex &parent = TzModelIndex()) const override;
    virtual TzModelIndex parent(const TzModelIndex &child) const override;
    virtual int rowCount(const TzModelIndex &parent = TzModelIndex()) const override;
    virtual int columnCount(const TzModelIndex &parent = TzModelIndex()) const override;

    bool setData(const TzModelIndex &index, const std::any &value,
                 int role = TzItemDataRole::EditRole) override;
    std::any data(const TzModelIndex &index, int role = TzItemDataRole::DisplayRole) const override;

    virtual TzTreeNodePtr nodeForIndex(const TzModelIndex &index) const;
    virtual TzModelIndex indexForNode(const TzTreeNodePtr &node, int column = 0) const;

protected:
    TzTreeModel(TzTreeModelPrivate &dd);

private:
    void connectNode(const TzTreeNodePtr &node);
    void disconnectNode(const TzTreeNodePtr &node);
    void connectRecursively(const TzTreeNodePtr &node);
    void disconnectRecursively(const TzTreeNodePtr &node);
};

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

inline TzTreeModelPrivate::TzTreeModelPrivate()
    : root(TzTreeNode::createFolder(""))
{
}

inline TzTreeModel::TzTreeModel(const TzTreeNodePtr &node)
    : TzTreeModel(*new TzTreeModelPrivate)
{
    TZ_D(TzTreeModel);
    if (d->root) {
        node->setParent(d->root);
        connectRecursively(d->root);
    }
}

inline TzTreeModel::TzTreeModel(const std::vector<TzTreeNodePtr> &nodes)
    : TzTreeModel(*new TzTreeModelPrivate)
{
    TZ_D(TzTreeModel);
    for (const TzTreeNodePtr &node : nodes)
        node->setParent(d->root);
    connectRecursively(d->root);
}

inline TzTreeModel::~TzTreeModel()
{
    TZ_D(TzTreeModel);
    if (d->root)
        disconnectRecursively(d->root);
}

inline TzTreeNodePtr TzTreeModel::invisibleRoot() const
{
    TZ_D(const TzTreeModel);
    return d->root;
}

inline TzModelIndex TzTreeModel::index(int row, int column, const TzModelIndex &parent) const
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

inline TzModelIndex TzTreeModel::parent(const TzModelIndex &child) const
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

inline int TzTreeModel::rowCount(const TzModelIndex &parent) const
{
    TZ_D(const TzTreeModel);
    if (parent.column() > 0)
        return 0;

    TzTreeNodePtr parentNode = parent.isValid() ? nodeForIndex(parent) : d->root;
    return parentNode ? static_cast<int>(parentNode->childCount()) : 0;
}

inline int TzTreeModel::columnCount(const TzModelIndex &parent) const
{
    (void) parent;
    return 1;
}

inline bool TzTreeModel::setData(const TzModelIndex &index, const std::any &value, int role)
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

inline std::any TzTreeModel::data(const TzModelIndex &index, int role) const
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

inline TzTreeNodePtr TzTreeModel::nodeForIndex(const TzModelIndex &index) const
{
    TZ_D(const TzTreeModel);
    if (!index.isValid())
        return d->root;

    return static_cast<TzTreeNode *>(index.internalPointer())->shared_from_this();
}

inline TzModelIndex TzTreeModel::indexForNode(const TzTreeNodePtr &node, int column) const
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

inline TzTreeModel::TzTreeModel(TzTreeModelPrivate &dd)
    : TzAbstractItemModel(dd)
{
}

inline void TzTreeModel::connectNode(const TzTreeNodePtr &node)
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
        node->events().on("childRemoved",
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

inline void TzTreeModel::disconnectNode(const TzTreeNodePtr &node)
{
    TZ_D(TzTreeModel);
    if (!node)
        return;

    d->connections.erase(node.get());
}

inline void TzTreeModel::connectRecursively(const TzTreeNodePtr &node)
{
    if (!node)
        return;

    connectNode(node);

    for (const auto &child : node->children())
        connectRecursively(child);
}

inline void TzTreeModel::disconnectRecursively(const TzTreeNodePtr &node)
{
    if (!node)
        return;

    for (const auto &child : node->children())
        disconnectRecursively(child);

    disconnectNode(node);
}
