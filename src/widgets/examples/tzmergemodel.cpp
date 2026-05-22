#include "tzmergemodel.hpp"

// ---------------------------------------------------------------------------

TzMergeModel::TzMergeModel(std::string rootName, std::string categoryName)
    : m_rootName(std::move(rootName))
    , m_categoryName(std::move(categoryName))
{}

TzMergeModel::~TzMergeModel() = default;

// ---------------------------------------------------------------------------

void TzMergeModel::addSubModel(std::string name, TzAbstractItemModel *model)
{
    if (!model)
        return;

    const int subIdx = static_cast<int>(m_subs.size());

    SubModelEntry entry;
    entry.name  = std::move(name);
    entry.model = model;
    entry.groupNode = std::make_unique<MergeNode>(
        MergeNode{MergeNode::Group, subIdx, {}});

    // ---- rowsAboutToBeInserted ----
    // Only forwarded for root-level insertions (srcParent invalid).
    // Nested insertions are forwarded when srcParent maps to a known cell.
    entry.listeners.push_back(model->events().on("rowsAboutToBeInserted",
        [this, subIdx](const TzModelIndex &srcParent, int first, int last) {
            SubModelEntry &e = m_subs[static_cast<std::size_t>(subIdx)];
            TzModelIndex proxyParent;
            if (!srcParent.isValid()) {
                proxyParent = makeIndex(subIdx, 0, e.groupNode.get());
            } else {
                proxyParent = mapFromSource(subIdx, srcParent);
                if (!proxyParent.isValid()) return; // not yet indexed – skip
            }
            beginInsertRows(proxyParent, first, last);
            e.insertPending = true;
        }));

    entry.listeners.push_back(model->events().on("rowsInserted",
        [this, subIdx](const TzModelIndex & /*srcParent*/, int, int) {
            SubModelEntry &e = m_subs[static_cast<std::size_t>(subIdx)];
            if (e.insertPending) {
                endInsertRows();
                e.insertPending = false;
            }
        }));

    // ---- rowsAboutToBeRemoved ----
    entry.listeners.push_back(model->events().on("rowsAboutToBeRemoved",
        [this, subIdx](const TzModelIndex &srcParent, int first, int last) {
            SubModelEntry &e = m_subs[static_cast<std::size_t>(subIdx)];
            TzModelIndex proxyParent;
            if (!srcParent.isValid()) {
                proxyParent = makeIndex(subIdx, 0, e.groupNode.get());
            } else {
                proxyParent = mapFromSource(subIdx, srcParent);
                if (!proxyParent.isValid()) return;
            }
            beginRemoveRows(proxyParent, first, last);
            e.removePending = true;
        }));

    entry.listeners.push_back(model->events().on("rowsRemoved",
        [this, subIdx](const TzModelIndex & /*srcParent*/, int, int) {
            SubModelEntry &e = m_subs[static_cast<std::size_t>(subIdx)];
            if (e.removePending) {
                clearSourceCells(subIdx);
                endRemoveRows();
                e.removePending = false;
            }
        }));

    // ---- dataChanged ----
    entry.listeners.push_back(model->events().on("dataChanged",
        [this, subIdx](const TzModelIndex &srcTL,
                       const TzModelIndex &srcBR,
                       const std::vector<int> &roles) {
            TzModelIndex proxyTL = mapFromSource(subIdx, srcTL);
            TzModelIndex proxyBR = mapFromSource(subIdx, srcBR);
            if (proxyTL.isValid())
                dataChanged(proxyTL, proxyBR, roles);
        }));

    // ---- modelReset ----
    entry.listeners.push_back(model->events().on("modelReset",
        [this, subIdx]() {
            clearSourceCells(subIdx);
            beginResetModel();
            endResetModel();
        }));

    m_subs.push_back(std::move(entry));
}

// ---------------------------------------------------------------------------
// Helpers

TzModelIndex TzMergeModel::makeIndex(int row, int col, MergeNode *node) const
{
    return createIndex(row, col, node);
}

TzMergeModel::MergeNode *TzMergeModel::nodeFor(const TzModelIndex &idx)
{
    if (!idx.isValid())
        return nullptr;
    return static_cast<MergeNode *>(idx.internalPointer());
}

TzMergeModel::MergeNode *
TzMergeModel::getOrCreateSourceCell(int subIdx, const TzModelIndex &srcIdx) const
{
    if (!srcIdx.isValid())
        return nullptr;
    auto key = std::make_pair(subIdx, srcIdx.internalId());
    auto it = m_cells.find(key);
    if (it != m_cells.end())
        return it->second.get();
    auto cell = std::make_unique<MergeNode>(
        MergeNode{MergeNode::Source, subIdx, srcIdx});
    auto *ptr = cell.get();
    m_cells.emplace(key, std::move(cell));
    return ptr;
}

void TzMergeModel::clearSourceCells(int subIdx)
{
    for (auto it = m_cells.begin(); it != m_cells.end(); ) {
        if (it->first.first == subIdx)
            it = m_cells.erase(it);
        else
            ++it;
    }
}

// ---------------------------------------------------------------------------
// TzAbstractItemModel interface

TzModelIndex TzMergeModel::index(int row, int col,
                                  const TzModelIndex &parent) const
{
    if (!hasIndex(row, col, parent))
        return {};

    MergeNode *pn = nodeFor(parent);

    // invisible root → home
    if (!pn) {
        return row == 0 ? makeIndex(0, col,
            const_cast<MergeNode *>(&m_homeNode)) : TzModelIndex{};
    }

    switch (pn->kind) {
    case MergeNode::Home:
        return row == 0 ? makeIndex(0, col,
            const_cast<MergeNode *>(&m_materialsNode)) : TzModelIndex{};

    case MergeNode::Materials: {
        int si = row;
        return makeIndex(row, col,
            m_subs[static_cast<std::size_t>(si)].groupNode.get());
    }

    case MergeNode::Group: {
        int si = pn->subModelIdx;
        TzModelIndex srcIdx = m_subs[static_cast<std::size_t>(si)].model->index(row, col);
        if (!srcIdx.isValid())
            return {};
        return makeIndex(row, col, getOrCreateSourceCell(si, srcIdx));
    }

    case MergeNode::Source: {
        int si = pn->subModelIdx;
        TzModelIndex srcIdx =
            m_subs[static_cast<std::size_t>(si)].model->index(row, col, pn->srcIdx);
        if (!srcIdx.isValid())
            return {};
        return makeIndex(row, col, getOrCreateSourceCell(si, srcIdx));
    }
    }
    return {};
}

TzModelIndex TzMergeModel::parent(const TzModelIndex &child) const
{
    MergeNode *n = nodeFor(child);
    if (!n)
        return {};

    switch (n->kind) {
    case MergeNode::Home:
        return {}; // invisible root

    case MergeNode::Materials:
        return makeIndex(0, 0, const_cast<MergeNode *>(&m_homeNode));

    case MergeNode::Group:
        return makeIndex(0, 0, const_cast<MergeNode *>(&m_materialsNode));

    case MergeNode::Source: {
        int si = n->subModelIdx;
        TzModelIndex srcParent =
            m_subs[static_cast<std::size_t>(si)].model->parent(n->srcIdx);
        if (!srcParent.isValid()) {
            // top-level source item → parent is the group node
            return makeIndex(si, 0,
                m_subs[static_cast<std::size_t>(si)].groupNode.get());
        }
        // nested source item → parent is another source cell
        return makeIndex(srcParent.row(), 0,
            getOrCreateSourceCell(si, srcParent));
    }
    }
    return {};
}

int TzMergeModel::rowCount(const TzModelIndex &parent) const
{
    MergeNode *n = nodeFor(parent);

    if (!n)
        return 1; // invisible root has one child: home

    switch (n->kind) {
    case MergeNode::Home:
        return 1; // materials

    case MergeNode::Materials:
        return static_cast<int>(m_subs.size());

    case MergeNode::Group: {
        int si = n->subModelIdx;
        return m_subs[static_cast<std::size_t>(si)].model->rowCount();
    }

    case MergeNode::Source: {
        int si = n->subModelIdx;
        return m_subs[static_cast<std::size_t>(si)].model->rowCount(n->srcIdx);
    }
    }
    return 0;
}

int TzMergeModel::columnCount(const TzModelIndex & /*parent*/) const
{
    return 1;
}

std::any TzMergeModel::data(const TzModelIndex &idx, int role) const
{
    if (role != TzItemDataRole::DisplayRole)
        return {};

    MergeNode *n = nodeFor(idx);
    if (!n)
        return {};

    switch (n->kind) {
    case MergeNode::Home:      return m_rootName;
    case MergeNode::Materials: return m_categoryName;
    case MergeNode::Group:
        return m_subs[static_cast<std::size_t>(n->subModelIdx)].name;
    case MergeNode::Source: {
        int si = n->subModelIdx;
        return m_subs[static_cast<std::size_t>(si)].model->data(
            n->srcIdx, role);
    }
    }
    return {};
}

TzModelIndex TzMergeModel::mapFromSource(int subIdx, const TzModelIndex &srcIdx) const
{
    if (!srcIdx.isValid()
        || subIdx < 0
        || subIdx >= static_cast<int>(m_subs.size()))
        return {};
    MergeNode *cell = getOrCreateSourceCell(subIdx, srcIdx);
    return cell ? makeIndex(srcIdx.row(), srcIdx.column(), cell) : TzModelIndex{};
}

TzModelIndex TzMergeModel::mapToSource(const TzModelIndex &proxyIdx) const
{
    MergeNode *n = nodeFor(proxyIdx);
    if (n && n->kind == MergeNode::Source)
        return n->srcIdx;
    return {};
}
