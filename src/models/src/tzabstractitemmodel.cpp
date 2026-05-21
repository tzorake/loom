#include <loom/tzabstractitemmodel.hpp>
#include <tzabstractitemmodel_p.hpp>

#include <loom/tzassert.hpp>
#include <loom/tzdebug.hpp>
#include <loom/tzglobalstatic.hpp>

#include <algorithm>

TzPersistentModelIndexData *TzPersistentModelIndexData::create(const TzModelIndex &index)
{
    TZ_ASSERT(index.isValid()); // we will _never_ insert an invalid index in the list
    TzPersistentModelIndexData *d = nullptr;
    TzAbstractItemModel *model = const_cast<TzAbstractItemModel *>(index.model());
    auto &indexes = model->d_func()->persistent.indexes;
    const auto it = indexes.find(index);
    if (it != indexes.cend()) {
        d = it->second;
    } else {
        d = new TzPersistentModelIndexData(index);
        indexes.insert({index, d});
    }
    TZ_ASSERT(d);
    return d;
}

void TzPersistentModelIndexData::destroy(TzPersistentModelIndexData *data)
{
    TZ_ASSERT(data);
    TZ_ASSERT(data->ref.load(std::memory_order_relaxed) == 0);
    TzAbstractItemModel *model = const_cast<TzAbstractItemModel *>(data->index.model());
    // a valid persistent model index with a null model pointer can only happen if the model was destroyed
    if (model) {
        TzAbstractItemModelPrivate *p = model->d_func();
        TZ_ASSERT(p);
        p->removePersistentIndexData(data);
    }
    delete data;
}

TzPersistentModelIndex::TzPersistentModelIndex()
    : d(nullptr)
{
}

TzPersistentModelIndex::TzPersistentModelIndex(const TzPersistentModelIndex &other)
    : d(other.d)
{
    if (d)
        d->ref.fetch_add(1, std::memory_order_relaxed);
}

TzPersistentModelIndex::TzPersistentModelIndex(const TzModelIndex &index)
    : d(nullptr)
{
    if (index.isValid()) {
        d = TzPersistentModelIndexData::create(index);
        d->ref.fetch_add(1, std::memory_order_relaxed);
    }
}

TzPersistentModelIndex::~TzPersistentModelIndex()
{
    if (d && d->ref.fetch_sub(1, std::memory_order_acq_rel) == 1) {
        TzPersistentModelIndexData::destroy(d);
        d = nullptr;
    }
}

TzPersistentModelIndex &TzPersistentModelIndex::operator=(const TzPersistentModelIndex &other)
{
    if (d == other.d)
        return *this;
    if (d && d->ref.fetch_sub(1, std::memory_order_acq_rel) == 1)
        TzPersistentModelIndexData::destroy(d);
    d = other.d;
    if (d) d->ref.fetch_add(1, std::memory_order_relaxed);
    return *this;
}

TzPersistentModelIndex &TzPersistentModelIndex::operator=(const TzModelIndex &other)
{
    if (d && d->ref.fetch_sub(1, std::memory_order_acq_rel) == 1)
        TzPersistentModelIndexData::destroy(d);
    if (other.isValid()) {
        d = TzPersistentModelIndexData::create(other);
        if (d) d->ref.fetch_add(1, std::memory_order_relaxed);
    } else {
        d = nullptr;
    }
    return *this;
}

TzPersistentModelIndex::operator TzModelIndex() const
{
    if (d)
        return d->index;
    return TzModelIndex();
}

bool TzPersistentModelIndex::operator<(const TzPersistentModelIndex &other) const noexcept
{
    return d < other.d;
}

bool TzPersistentModelIndex::operator==(const TzPersistentModelIndex &other) const noexcept
{
    return d == other.d;
}

int TzPersistentModelIndex::row() const
{
    if (d)
        return d->index.row();
    return -1;
}

int TzPersistentModelIndex::column() const
{
    if (d)
        return d->index.column();
    return -1;
}

void *TzPersistentModelIndex::internalPointer() const
{
    if (d)
        return d->index.internalPointer();
    return nullptr;
}

const void *TzPersistentModelIndex::constInternalPointer() const
{
    if (d)
        return d->index.constInternalPointer();
    return nullptr;
}

uintptr_t TzPersistentModelIndex::internalId() const
{
    if (d)
        return d->index.internalId();
    return 0;
}

TzModelIndex TzPersistentModelIndex::parent() const
{
    if (d)
        return d->index.parent();
    return TzModelIndex();
}

TzModelIndex TzPersistentModelIndex::sibling(int row, int column) const
{
    if (d)
        return d->index.sibling(row, column);
    return TzModelIndex();
}

std::any TzPersistentModelIndex::data(int role) const
{
    if (d)
        return d->index.data(role);
    return std::any();
}

TzItemFlags TzPersistentModelIndex::flags() const
{
    if (d)
        return d->index.flags();
    return TzItemFlag::NoItemFlags;
}

const TzAbstractItemModel *TzPersistentModelIndex::model() const
{
    if (d)
        return d->index.model();
    return nullptr;
}

bool TzPersistentModelIndex::isValid() const
{
    return d && d->index.isValid();
}

class TzEmptyItemModel : public TzAbstractItemModel
{
public:
    explicit TzEmptyItemModel() : TzAbstractItemModel() {}
    TzModelIndex index(int, int, const TzModelIndex &) const override { return TzModelIndex(); }
    TzModelIndex parent(const TzModelIndex &) const override { return TzModelIndex(); }
    int rowCount(const TzModelIndex &) const override { return 0; }
    int columnCount(const TzModelIndex &) const override { return 0; }
    bool hasChildren(const TzModelIndex &) const override { return false; }
    std::any data(const TzModelIndex &, int) const override { return std::any(); }
};

TZ_GLOBAL_STATIC(TzEmptyItemModel, tzEmptyModel)

TzAbstractItemModelPrivate::TzAbstractItemModelPrivate()
{
}

TzAbstractItemModelPrivate::~TzAbstractItemModelPrivate()
{
}

TzAbstractItemModel *TzAbstractItemModelPrivate::staticEmptyModel()
{
    return tzEmptyModel();
}

void TzAbstractItemModelPrivate::invalidatePersistentIndexes()
{
    for (auto &[_k, data] : std::as_const(persistent.indexes))
        data->index = TzModelIndex();
    persistent.indexes.clear();
}

void TzAbstractItemModelPrivate::invalidatePersistentIndex(const TzModelIndex &index) {
    const auto it = persistent.indexes.find(index);
    if (it != persistent.indexes.end()) {
        TzPersistentModelIndexData *data = it->second;
        persistent.indexes.erase(it);
        data->index = TzModelIndex();
    }
}

using DefaultRoleNames = std::unordered_map<int, std::string>;
TZ_GLOBAL_STATIC(DefaultRoleNames, tzDefaultRoleNames,
    {
        { TzItemDataRole::DisplayRole, "display" },
        { TzItemDataRole::DecorationRole, "decoration" },
        { TzItemDataRole::EditRole, "edit" },
        { TzItemDataRole::ToolTipRole, "toolTip" },
        { TzItemDataRole::StatusTipRole, "statusTip" },
        { TzItemDataRole::WhatsThisRole, "whatsThis" },
    })

const std::unordered_map<int,std::string> &TzAbstractItemModelPrivate::defaultRoleNames()
{
    return *tzDefaultRoleNames();
}

void TzAbstractItemModelPrivate::removePersistentIndexData(TzPersistentModelIndexData *data)
{
    if (data->index.isValid()) {
        int removed = 0;
        auto range = persistent.indexes.equal_range(data->index);
        for (auto it = range.first; it != range.second; ) {
            if (it->second == data) {
                it = persistent.indexes.erase(it);
                ++removed;
            } else {
                ++it;
            }
        }
        TZ_ASSERT_X(removed == 1, "TzPersistentModelIndex::~TzPersistentModelIndex",
                   "persistent model indexes corrupted");
        TZ_UNUSED(removed);
    }
    // make sure our optimization still works
    for (int i = (int)persistent.moved.size() - 1; i >= 0; --i) {
        auto &movedVec = persistent.moved[i];
        auto it = std::find(movedVec.begin(), movedVec.end(), data);
        if (it != movedVec.end())
            movedVec.erase(it);
    }
    // update the references to invalidated persistent indexes
    for (int i = (int)persistent.invalidated.size() - 1; i >= 0; --i) {
        auto &invalidatedVec = persistent.invalidated[i];
        auto it = std::find(invalidatedVec.begin(), invalidatedVec.end(), data);
        if (it != invalidatedVec.end())
            invalidatedVec.erase(it);
    }
}

void TzAbstractItemModelPrivate::rowsAboutToBeInserted(const TzModelIndex &parent, int first, int last)
{
    TZ_Q(TzAbstractItemModel);
    TZ_UNUSED(last);
    std::vector<TzPersistentModelIndexData *> persistentMoved;
    if (first < q->rowCount(parent)) {
        for (auto &[_k, data] : std::as_const(persistent.indexes)) {
            const TzModelIndex &index = data->index;
            if (index.row() >= first && index.isValid() && index.parent() == parent) {
                persistentMoved.push_back(data);
            }
        }
    }
    persistent.moved.push_back(persistentMoved);
}

void TzAbstractItemModelPrivate::rowsInserted(const TzModelIndex &parent, int first, int last)
{
    const std::vector<TzPersistentModelIndexData *> persistentMoved = persistent.moved.back(); persistent.moved.pop_back();
    const int count = (last - first) + 1; // it is important to only use the delta, because the change could be nested
    for (auto *data : persistentMoved) {
        TzModelIndex old = data->index;
        persistent.indexes.erase(persistent.indexes.find(old));
        data->index = q_func()->index(old.row() + count, old.column(), parent);
        if (data->index.isValid()) {
            persistent.insertMultiAtEnd(data->index, data);
        } else {
            tzWarning() << "TzAbstractItemModel::endInsertRows: Invalid index in model";
        }
    }
}

void TzAbstractItemModelPrivate::itemsAboutToBeMoved(const TzModelIndex &srcParent, int srcFirst, int srcLast, const TzModelIndex &destinationParent, int destinationChild, TzOrientation orientation)
{
    std::vector<TzPersistentModelIndexData *> persistentMovedExplicitly;
    std::vector<TzPersistentModelIndexData *> persistentMovedInSource;
    std::vector<TzPersistentModelIndexData *> persistentMovedInDestination;

    const bool sameParent = (srcParent == destinationParent);
    const bool movingUp = (srcFirst > destinationChild);

    for (auto &[_k, data] : std::as_const(persistent.indexes)) {
        const TzModelIndex &index = data->index;
        const TzModelIndex &parent = index.parent();
        const bool isSourceIndex = (parent == srcParent);
        const bool isDestinationIndex = (parent == destinationParent);

        int childPosition;
        if (orientation == TzOrientation::Vertical)
            childPosition = index.row();
        else
            childPosition = index.column();

        if (!index.isValid() || !(isSourceIndex || isDestinationIndex ) )
            continue;

        if (!sameParent && isDestinationIndex) {
            if (childPosition >= destinationChild)
                persistentMovedInDestination.push_back(data);
            continue;
        }

        if (sameParent && movingUp && childPosition < destinationChild)
            continue;

        if (sameParent && !movingUp && childPosition < srcFirst )
            continue;

        if (!sameParent && childPosition < srcFirst)
            continue;

        if (sameParent && (childPosition > srcLast) && (childPosition >= destinationChild ))
            continue;

        if ((childPosition <= srcLast) && (childPosition >= srcFirst)) {
            persistentMovedExplicitly.push_back(data);
        } else {
            persistentMovedInSource.push_back(data);
        }
    }
    persistent.moved.push_back(persistentMovedExplicitly);
    persistent.moved.push_back(persistentMovedInSource);
    persistent.moved.push_back(persistentMovedInDestination);
}

void TzAbstractItemModelPrivate::movePersistentIndexes(const std::vector<TzPersistentModelIndexData *> &indexes, int change, const TzModelIndex &parent, TzOrientation orientation)
{
    for (auto *data : indexes) {
        int row = data->index.row();
        int column = data->index.column();

        if (TzOrientation::Vertical == orientation)
            row += change;
        else
            column += change;

        persistent.indexes.erase(persistent.indexes.find(data->index));
        data->index = q_func()->index(row, column, parent);
        if (data->index.isValid()) {
            persistent.insertMultiAtEnd(data->index, data);
        } else {
            tzWarning() << "TzAbstractItemModel::endMoveRows: Invalid index in model";
        }
    }
}

void TzAbstractItemModelPrivate::itemsMoved(const TzModelIndex &sourceParent, int sourceFirst, int sourceLast, const TzModelIndex &destinationParent, int destinationChild, TzOrientation orientation)
{
    const std::vector<TzPersistentModelIndexData *> movedInDestination = persistent.moved.back(); persistent.moved.pop_back();
    const std::vector<TzPersistentModelIndexData *> movedInSource = persistent.moved.back(); persistent.moved.pop_back();
    const std::vector<TzPersistentModelIndexData *> movedExplicitly = persistent.moved.back(); persistent.moved.pop_back();

    const bool sameParent = (sourceParent == destinationParent);
    const bool movingUp = (sourceFirst > destinationChild);

    const int explicitChange = (!sameParent || movingUp) ? destinationChild - sourceFirst : destinationChild - sourceLast - 1 ;
    const int sourceChange = (!sameParent || !movingUp) ? -1*(sourceLast - sourceFirst + 1) : sourceLast - sourceFirst + 1 ;
    const int destinationChange = sourceLast - sourceFirst + 1;

    movePersistentIndexes(movedExplicitly, explicitChange, destinationParent, orientation);
    movePersistentIndexes(movedInSource, sourceChange, sourceParent, orientation);
    movePersistentIndexes(movedInDestination, destinationChange, destinationParent, orientation);
}

void TzAbstractItemModelPrivate::rowsAboutToBeRemoved(const TzModelIndex &parent, int first, int last)
{
    std::vector<TzPersistentModelIndexData *> persistentMoved;
    std::vector<TzPersistentModelIndexData *> persistentInvalidated;
    // find the persistent indexes that are affected by the change, either by being in the removed subtree
    // or by being on the same level and below the removed rows
    for (auto &[_k, data] : std::as_const(persistent.indexes)) {
        bool levelChanged = false;
        TzModelIndex current = data->index;
        while (current.isValid()) {
            TzModelIndex currentParent = current.parent();
            if (currentParent == parent) { // on the same level as the change
                if (!levelChanged && current.row() > last) // below the removed rows
                    persistentMoved.push_back(data);
                else if (current.row() <= last && current.row() >= first) // in the removed subtree
                    persistentInvalidated.push_back(data);
                break;
            }
            current = currentParent;
            levelChanged = true;
        }
    }

    persistent.moved.push_back(std::move(persistentMoved));
    persistent.invalidated.push_back(std::move(persistentInvalidated));
}

void TzAbstractItemModelPrivate::rowsRemoved(const TzModelIndex &parent, int first, int last)
{
    const std::vector<TzPersistentModelIndexData *> persistentMoved = std::move(persistent.moved.back()); persistent.moved.pop_back();
    const int count = (last - first) + 1; // it is important to only use the delta, because the change could be nested
    for (auto *data : persistentMoved) {
        TzModelIndex old = data->index;
        persistent.indexes.erase(persistent.indexes.find(old));
        data->index = q_func()->index(old.row() - count, old.column(), parent);
        if (data->index.isValid()) {
            persistent.insertMultiAtEnd(data->index, data);
        } else {
            tzWarning() << "TzAbstractItemModel::endRemoveRows: Invalid index in model";
        }
    }
    const std::vector<TzPersistentModelIndexData *> persistentInvalidated = std::move(persistent.invalidated.back()); persistent.invalidated.pop_back();
    for (auto *data : persistentInvalidated) {
        auto it = persistent.indexes.find(data->index);
        if (it != persistent.indexes.end())
            persistent.indexes.erase(it);
        data->index = TzModelIndex();
    }
}

void TzAbstractItemModelPrivate::columnsAboutToBeInserted(const TzModelIndex &parent, int first, int last)
{
    TZ_Q(TzAbstractItemModel);
    TZ_UNUSED(last);
    std::vector<TzPersistentModelIndexData *> persistentMoved;
    if (first < q->columnCount(parent)) {
        for (auto &[_k, data] : std::as_const(persistent.indexes)) {
            const TzModelIndex &index = data->index;
            if (index.column() >= first && index.isValid() && index.parent() == parent)
                persistentMoved.push_back(data);
        }
    }
    persistent.moved.push_back(std::move(persistentMoved));
}

void TzAbstractItemModelPrivate::columnsInserted(const TzModelIndex &parent, int first, int last)
{
    const std::vector<TzPersistentModelIndexData *> persistentMoved = std::move(persistent.moved.back()); persistent.moved.pop_back();
    const int count = (last - first) + 1; // it is important to only use the delta, because the change could be nested
    for (auto *data : persistentMoved) {
        TzModelIndex old = data->index;
        persistent.indexes.erase(persistent.indexes.find(old));
        data->index = q_func()->index(old.row(), old.column() + count, parent);
        if (data->index.isValid()) {
            persistent.insertMultiAtEnd(data->index, data);
        } else {
            tzWarning() << "TzAbstractItemModel::endInsertColumns: Invalid index in model";
        }
    }
}

void TzAbstractItemModelPrivate::columnsAboutToBeRemoved(const TzModelIndex &parent, int first, int last)
{
    std::vector<TzPersistentModelIndexData *> persistentMoved;
    std::vector<TzPersistentModelIndexData *> persistentInvalidated;
    // find the persistent indexes that are affected by the change, either by being in the removed subtree
    // or by being on the same level and to the right of the removed columns
    for (auto &[_k, data] : std::as_const(persistent.indexes)) {
        bool levelChanged = false;
        TzModelIndex current = data->index;
        while (current.isValid()) {
            TzModelIndex currentParent = current.parent();
            if (currentParent == parent) { // on the same level as the change
                if (!levelChanged && current.column() > last) // right of the removed columns
                    persistentMoved.push_back(data);
                else if (current.column() <= last && current.column() >= first) // in the removed subtree
                    persistentInvalidated.push_back(data);
                break;
            }
            current = currentParent;
            levelChanged = true;
        }
    }

    persistent.moved.push_back(std::move(persistentMoved));
    persistent.invalidated.push_back(std::move(persistentInvalidated));
}

void TzAbstractItemModelPrivate::columnsRemoved(const TzModelIndex &parent, int first, int last)
{
    const std::vector<TzPersistentModelIndexData *> persistentMoved = std::move(persistent.moved.back()); persistent.moved.pop_back();
    const int count = (last - first) + 1; // it is important to only use the delta, because the change could be nested
    for (auto *data : persistentMoved) {
        TzModelIndex old = data->index;
        persistent.indexes.erase(persistent.indexes.find(old));
        data->index = q_func()->index(old.row(), old.column() - count, parent);
        if (data->index.isValid()) {
            persistent.insertMultiAtEnd(data->index, data);
        } else {
            tzWarning() << "TzAbstractItemModel::endRemoveColumns: Invalid index in model";
        }
    }
    const std::vector<TzPersistentModelIndexData *> persistentInvalidated = std::move(persistent.invalidated.back()); persistent.invalidated.pop_back();
    for (auto *data : persistentInvalidated) {
        auto index = persistent.indexes.find(data->index);
        if (index != persistent.indexes.cend())
            persistent.indexes.erase(index);
        data->index = TzModelIndex();
    }
}

void TzAbstractItemModel::resetInternalData()
{
}

TzAbstractItemModel::TzAbstractItemModel()
    : d_ptr(new TzAbstractItemModelPrivate)
{
    d_ptr->q_ptr = this;
}

TzAbstractItemModel::TzAbstractItemModel(TzAbstractItemModelPrivate &dd)
    : d_ptr(&dd)
{
    d_ptr->q_ptr = this;
}

TzAbstractItemModel::~TzAbstractItemModel()
{
    d_func()->invalidatePersistentIndexes();
}

bool TzAbstractItemModel::hasIndex(int row, int column, const TzModelIndex &parent) const
{
    if (row < 0 || column < 0)
        return false;
    return row < rowCount(parent) && column < columnCount(parent);
}

bool TzAbstractItemModel::hasChildren(const TzModelIndex &parent) const
{
    return (rowCount(parent) > 0) && (columnCount(parent) > 0);
}

TzModelIndex TzAbstractItemModel::sibling(int row, int column, const TzModelIndex &idx) const
{
    return (row == idx.row() && column == idx.column()) ? idx : index(row, column, parent(idx));
}

bool TzAbstractItemModel::setData(const TzModelIndex &index, const std::any &value, int role)
{
    TZ_UNUSED(index);
    TZ_UNUSED(value);
    TZ_UNUSED(role);
    return false;
}

bool TzAbstractItemModel::insertRows(int, int, const TzModelIndex &)
{
    return false;
}

bool TzAbstractItemModel::insertColumns(int, int, const TzModelIndex &)
{
    return false;
}

bool TzAbstractItemModel::removeRows(int, int, const TzModelIndex &)
{
    return false;
}

bool TzAbstractItemModel::removeColumns(int, int, const TzModelIndex &)
{
    return false;
}

bool TzAbstractItemModel::moveRows(const TzModelIndex &, int , int , const TzModelIndex &, int)
{
    return false;
}

bool TzAbstractItemModel::moveColumns(const TzModelIndex &, int , int , const TzModelIndex &, int)
{
    return false;
}

void TzAbstractItemModel::fetchMore(const TzModelIndex &)
{
    // do nothing
}

bool TzAbstractItemModel::canFetchMore(const TzModelIndex &) const
{
    return false;
}

TzItemFlags TzAbstractItemModel::flags(const TzModelIndex &index) const
{
    TZ_D(const TzAbstractItemModel);
    if (!d->indexValid(index))
        return TzItemFlag::NoItemFlags;
    return TzItemFlag::ItemIsSelectable | TzItemFlag::ItemIsEnabled;
}

void TzAbstractItemModel::sort(int column, TzSortOrder order)
{
    TZ_UNUSED(column);
    TZ_UNUSED(order);
    // do nothing
}

TzModelIndex TzAbstractItemModel::buddy(const TzModelIndex &index) const
{
    return index;
}

std::unordered_map<int,std::string> TzAbstractItemModel::roleNames() const
{
    return TzAbstractItemModelPrivate::defaultRoleNames();
}

bool TzAbstractItemModel::submit()
{
    return true;
}


void TzAbstractItemModel::revert()
{
    // do nothing
}


std::any TzAbstractItemModel::headerData(int section, TzOrientation orientation, int role) const
{
    TZ_UNUSED(orientation);
    if (role == TzItemDataRole::DisplayRole)
        return section + 1;
    return std::any();
}


bool TzAbstractItemModel::setHeaderData(int section, TzOrientation orientation, const std::any &value, int role)
{
    TZ_UNUSED(section);
    TZ_UNUSED(orientation);
    TZ_UNUSED(value);
    TZ_UNUSED(role);
    return false;
}

// ── Signal implementations ──────────────────────────────────────────────────

void TzAbstractItemModel::dataChanged(const TzModelIndex &topLeft, const TzModelIndex &bottomRight, const std::vector<int> &roles)
{
    emitter.emit("dataChanged", topLeft, bottomRight, roles);
}

void TzAbstractItemModel::headerDataChanged(TzOrientation orientation, int first, int last)
{
    emitter.emit("headerDataChanged", orientation, first, last);
}

void TzAbstractItemModel::layoutChanged(const std::vector<TzPersistentModelIndex> &parents, TzAbstractItemModel::LayoutChangeHint hint)
{
    emitter.emit("layoutChanged", parents, hint);
}

void TzAbstractItemModel::layoutAboutToBeChanged(const std::vector<TzPersistentModelIndex> &parents, TzAbstractItemModel::LayoutChangeHint hint)
{
    emitter.emit("layoutAboutToBeChanged", parents, hint);
}

void TzAbstractItemModel::rowsAboutToBeInserted(const TzModelIndex &parent, int first, int last)
{
    emitter.emit("rowsAboutToBeInserted", parent, first, last);
}

void TzAbstractItemModel::rowsInserted(const TzModelIndex &parent, int first, int last)
{
    emitter.emit("rowsInserted", parent, first, last);
}

void TzAbstractItemModel::rowsAboutToBeRemoved(const TzModelIndex &parent, int first, int last)
{
    emitter.emit("rowsAboutToBeRemoved", parent, first, last);
}

void TzAbstractItemModel::rowsRemoved(const TzModelIndex &parent, int first, int last)
{
    emitter.emit("rowsRemoved", parent, first, last);
}

void TzAbstractItemModel::columnsAboutToBeInserted(const TzModelIndex &parent, int first, int last)
{
    emitter.emit("columnsAboutToBeInserted", parent, first, last);
}

void TzAbstractItemModel::columnsInserted(const TzModelIndex &parent, int first, int last)
{
    emitter.emit("columnsInserted", parent, first, last);
}

void TzAbstractItemModel::columnsAboutToBeRemoved(const TzModelIndex &parent, int first, int last)
{
    emitter.emit("columnsAboutToBeRemoved", parent, first, last);
}

void TzAbstractItemModel::columnsRemoved(const TzModelIndex &parent, int first, int last)
{
    emitter.emit("columnsRemoved", parent, first, last);
}

void TzAbstractItemModel::modelAboutToBeReset()
{
    emitter.emit("modelAboutToBeReset");
}

void TzAbstractItemModel::modelReset()
{
    emitter.emit("modelReset");
}

void TzAbstractItemModel::rowsAboutToBeMoved(const TzModelIndex &sourceParent, int sourceStart, int sourceEnd, const TzModelIndex &destinationParent, int destinationRow)
{
    emitter.emit("rowsAboutToBeMoved", sourceParent, sourceStart, sourceEnd, destinationParent, destinationRow);
}

void TzAbstractItemModel::rowsMoved(const TzModelIndex &sourceParent, int sourceStart, int sourceEnd, const TzModelIndex &destinationParent, int destinationRow)
{
    emitter.emit("rowsMoved", sourceParent, sourceStart, sourceEnd, destinationParent, destinationRow);
}

void TzAbstractItemModel::columnsAboutToBeMoved(const TzModelIndex &sourceParent, int sourceStart, int sourceEnd, const TzModelIndex &destinationParent, int destinationColumn)
{
    emitter.emit("columnsAboutToBeMoved", sourceParent, sourceStart, sourceEnd, destinationParent, destinationColumn);
}

void TzAbstractItemModel::columnsMoved(const TzModelIndex &sourceParent, int sourceStart, int sourceEnd, const TzModelIndex &destinationParent, int destinationColumn)
{
    emitter.emit("columnsMoved", sourceParent, sourceStart, sourceEnd, destinationParent, destinationColumn);
}

// ── Begin/End row operations ─────────────────────────────────────────────────

void TzAbstractItemModel::beginInsertRows(const TzModelIndex &parent, int first, int last)
{
    TZ_ASSERT(first >= 0);
    TZ_ASSERT(first <= rowCount(parent)); // == is allowed, to insert at the end
    TZ_ASSERT(last >= first);
    TZ_D(TzAbstractItemModel);
    d->changes.push_back(TzAbstractItemModelPrivate::Change(parent, first, last));
    rowsAboutToBeInserted(parent, first, last);
    d->rowsAboutToBeInserted(parent, first, last);
}

void TzAbstractItemModel::endInsertRows()
{
    TZ_D(TzAbstractItemModel);
    TzAbstractItemModelPrivate::Change change = d->changes.back(); d->changes.pop_back();
    d->rowsInserted(change.parent, change.first, change.last);
    rowsInserted(change.parent, change.first, change.last);
}

void TzAbstractItemModel::beginRemoveRows(const TzModelIndex &parent, int first, int last)
{
    TZ_ASSERT(first >= 0);
    TZ_ASSERT(last >= first);
    TZ_ASSERT(last < rowCount(parent));
    TZ_D(TzAbstractItemModel);
    d->changes.push_back(TzAbstractItemModelPrivate::Change(parent, first, last));
    rowsAboutToBeRemoved(parent, first, last);
    d->rowsAboutToBeRemoved(parent, first, last);
}

void TzAbstractItemModel::endRemoveRows()
{
    TZ_D(TzAbstractItemModel);
    TzAbstractItemModelPrivate::Change change = d->changes.back(); d->changes.pop_back();
    d->rowsRemoved(change.parent, change.first, change.last);
    rowsRemoved(change.parent, change.first, change.last);
}

bool TzAbstractItemModelPrivate::allowMove(const TzModelIndex &srcParent, int start, int end, const TzModelIndex &destinationParent, int destinationStart, TzOrientation orientation)
{
    // Don't move the range within itself.
    if (destinationParent == srcParent)
        return !(destinationStart >= start && destinationStart <= end + 1);

    TzModelIndex destinationAncestor = destinationParent;
    int pos = (TzOrientation::Vertical == orientation) ? destinationAncestor.row() : destinationAncestor.column();
    while (true) {
        if (destinationAncestor == srcParent) {
            if (pos >= start && pos <= end)
                return false;
            break;
        }

        if (!destinationAncestor.isValid())
          break;

        pos = (TzOrientation::Vertical == orientation) ? destinationAncestor.row() : destinationAncestor.column();
        destinationAncestor = destinationAncestor.parent();
    }

    return true;
}

void TzAbstractItemModelPrivate::executePendingOperations() const
{
}

bool TzAbstractItemModel::beginMoveRows(const TzModelIndex &sourceParent, int sourceFirst, int sourceLast, const TzModelIndex &destinationParent, int destinationChild)
{
    TZ_ASSERT(sourceFirst >= 0);
    TZ_ASSERT(sourceLast >= sourceFirst);
    TZ_ASSERT(destinationChild >= 0);
    TZ_D(TzAbstractItemModel);

    if (!d->allowMove(sourceParent, sourceFirst, sourceLast, destinationParent, destinationChild, TzOrientation::Vertical))
        return false;

    TzAbstractItemModelPrivate::Change sourceChange(sourceParent, sourceFirst, sourceLast);
    sourceChange.needsAdjust = sourceParent.isValid() && sourceParent.row() >= destinationChild && sourceParent.parent() == destinationParent;
    d->changes.push_back(sourceChange);
    int destinationLast = destinationChild + (sourceLast - sourceFirst);
    TzAbstractItemModelPrivate::Change destinationChange(destinationParent, destinationChild, destinationLast);
    destinationChange.needsAdjust = destinationParent.isValid() && destinationParent.row() >= sourceLast && destinationParent.parent() == sourceParent;
    d->changes.push_back(destinationChange);

    rowsAboutToBeMoved(sourceParent, sourceFirst, sourceLast, destinationParent, destinationChild);
    d->itemsAboutToBeMoved(sourceParent, sourceFirst, sourceLast, destinationParent, destinationChild, TzOrientation::Vertical);
    return true;
}

void TzAbstractItemModel::endMoveRows()
{
    TZ_D(TzAbstractItemModel);

    TzAbstractItemModelPrivate::Change insertChange = d->changes.back(); d->changes.pop_back();
    TzAbstractItemModelPrivate::Change removeChange = d->changes.back(); d->changes.pop_back();

    TzModelIndex adjustedSource = removeChange.parent;
    TzModelIndex adjustedDestination = insertChange.parent;

    const int numMoved = removeChange.last - removeChange.first + 1;
    if (insertChange.needsAdjust)
      adjustedDestination = createIndex(adjustedDestination.row() - numMoved, adjustedDestination.column(), adjustedDestination.internalPointer());

    if (removeChange.needsAdjust)
      adjustedSource = createIndex(adjustedSource.row() + numMoved, adjustedSource.column(), adjustedSource.internalPointer());

    d->itemsMoved(adjustedSource, removeChange.first, removeChange.last, adjustedDestination, insertChange.first, TzOrientation::Vertical);

    rowsMoved(adjustedSource, removeChange.first, removeChange.last, adjustedDestination, insertChange.first);
}

void TzAbstractItemModel::beginInsertColumns(const TzModelIndex &parent, int first, int last)
{
    TZ_ASSERT(first >= 0);
    TZ_ASSERT(first <= columnCount(parent)); // == is allowed, to insert at the end
    TZ_ASSERT(last >= first);
    TZ_D(TzAbstractItemModel);
    d->changes.push_back(TzAbstractItemModelPrivate::Change(parent, first, last));
    columnsAboutToBeInserted(parent, first, last);
    d->columnsAboutToBeInserted(parent, first, last);
}

void TzAbstractItemModel::endInsertColumns()
{
    TZ_D(TzAbstractItemModel);
    TzAbstractItemModelPrivate::Change change = d->changes.back(); d->changes.pop_back();
    d->columnsInserted(change.parent, change.first, change.last);
    columnsInserted(change.parent, change.first, change.last);
}

void TzAbstractItemModel::beginRemoveColumns(const TzModelIndex &parent, int first, int last)
{
    TZ_ASSERT(first >= 0);
    TZ_ASSERT(last >= first);
    TZ_ASSERT(last < columnCount(parent));
    TZ_D(TzAbstractItemModel);
    d->changes.push_back(TzAbstractItemModelPrivate::Change(parent, first, last));
    columnsAboutToBeRemoved(parent, first, last);
    d->columnsAboutToBeRemoved(parent, first, last);
}

void TzAbstractItemModel::endRemoveColumns()
{
    TZ_D(TzAbstractItemModel);
    TzAbstractItemModelPrivate::Change change = d->changes.back(); d->changes.pop_back();
    d->columnsRemoved(change.parent, change.first, change.last);
    columnsRemoved(change.parent, change.first, change.last);
}

bool TzAbstractItemModel::beginMoveColumns(const TzModelIndex &sourceParent, int sourceFirst, int sourceLast, const TzModelIndex &destinationParent, int destinationChild)
{
    TZ_ASSERT(sourceFirst >= 0);
    TZ_ASSERT(sourceLast >= sourceFirst);
    TZ_ASSERT(destinationChild >= 0);
    TZ_D(TzAbstractItemModel);

    if (!d->allowMove(sourceParent, sourceFirst, sourceLast, destinationParent, destinationChild, TzOrientation::Horizontal))
        return false;

    TzAbstractItemModelPrivate::Change sourceChange(sourceParent, sourceFirst, sourceLast);
    sourceChange.needsAdjust = sourceParent.isValid() && sourceParent.row() >= destinationChild && sourceParent.parent() == destinationParent;
    d->changes.push_back(sourceChange);
    int destinationLast = destinationChild + (sourceLast - sourceFirst);
    TzAbstractItemModelPrivate::Change destinationChange(destinationParent, destinationChild, destinationLast);
    destinationChange.needsAdjust = destinationParent.isValid() && destinationParent.row() >= sourceLast && destinationParent.parent() == sourceParent;
    d->changes.push_back(destinationChange);

    columnsAboutToBeMoved(sourceParent, sourceFirst, sourceLast, destinationParent, destinationChild);
    d->itemsAboutToBeMoved(sourceParent, sourceFirst, sourceLast, destinationParent, destinationChild, TzOrientation::Horizontal);
    return true;
}

void TzAbstractItemModel::endMoveColumns()
{
    TZ_D(TzAbstractItemModel);

    TzAbstractItemModelPrivate::Change insertChange = d->changes.back(); d->changes.pop_back();
    TzAbstractItemModelPrivate::Change removeChange = d->changes.back(); d->changes.pop_back();

    TzModelIndex adjustedSource = removeChange.parent;
    TzModelIndex adjustedDestination = insertChange.parent;

    const int numMoved = removeChange.last - removeChange.first + 1;
    if (insertChange.needsAdjust)
      adjustedDestination = createIndex(adjustedDestination.row(), adjustedDestination.column() - numMoved, adjustedDestination.internalPointer());

    if (removeChange.needsAdjust)
      adjustedSource = createIndex(adjustedSource.row(), adjustedSource.column() + numMoved, adjustedSource.internalPointer());

    d->itemsMoved(adjustedSource, removeChange.first, removeChange.last, adjustedDestination, insertChange.first, TzOrientation::Horizontal);
    columnsMoved(adjustedSource, removeChange.first, removeChange.last, adjustedDestination, insertChange.first);
}

void TzAbstractItemModel::beginResetModel()
{
    TZ_D(TzAbstractItemModel);
    if (d->resetting) {
        tzWarning() << "beginResetModel called without calling endResetModel first";
        // Warn, but don't return early in case user code relies on the incorrect behavior.
    }

    tzDebug() << "beginResetModel called; about to modelAboutToBeReset";
    d->resetting = true;
    modelAboutToBeReset();
}

void TzAbstractItemModel::endResetModel()
{
    TZ_D(TzAbstractItemModel);
    if (!d->resetting) {
        tzWarning() << "endResetModel called without calling beginResetModel first";
        // Warn, but don't return early in case user code relies on the incorrect behavior.
    }

    tzDebug() << "endResetModel called; about to modelReset";
    d->invalidatePersistentIndexes();
    resetInternalData();
    d->resetting = false;
    modelReset();
}

void TzAbstractItemModel::changePersistentIndex(const TzModelIndex &from, const TzModelIndex &to)
{
    TZ_D(TzAbstractItemModel);
    if (d->persistent.indexes.empty())
        return;
    // find the data and reinsert it sorted
    const auto it = d->persistent.indexes.find(from);
    if (it != d->persistent.indexes.cend()) {
        TzPersistentModelIndexData *data = it->second;
        d->persistent.indexes.erase(it);
        data->index = to;
        if (to.isValid())
            d->persistent.insertMultiAtEnd(to, data);
    }
}

void TzAbstractItemModel::changePersistentIndexList(const TzModelIndexList &from, const TzModelIndexList &to)
{
    TZ_D(TzAbstractItemModel);
    if (d->persistent.indexes.empty())
        return;
    std::vector<TzPersistentModelIndexData *> toBeReinserted;
    toBeReinserted.reserve(to.size());
    for (int i = 0; i < (int)from.size(); ++i) {
        if (from.at(i) == to.at(i))
            continue;
        const auto it = d->persistent.indexes.find(from.at(i));
        if (it != d->persistent.indexes.cend()) {
            TzPersistentModelIndexData *data = it->second;
            d->persistent.indexes.erase(it);
            data->index = to.at(i);
            if (data->index.isValid())
                toBeReinserted.push_back(data);
        }
    }

    for (auto *data : std::as_const(toBeReinserted))
        d->persistent.insertMultiAtEnd(data->index, data);
}

TzModelIndexList TzAbstractItemModel::persistentIndexList() const
{
    TZ_D(const TzAbstractItemModel);
    TzModelIndexList result;
    result.reserve(d->persistent.indexes.size());
    for (auto &[_k, data] : std::as_const(d->persistent.indexes))
        result.push_back(data->index);
    return result;
}


bool TzAbstractItemModel::checkIndex(const TzModelIndex &index, CheckIndexOptions options) const
{
    if (!index.isValid()) {
        if (options & CheckIndexOption::IndexIsValid) {
            tzWarning() << "Index is not valid (expected valid)";
            return false;
        }
        return true;
    }

    if (index.model() != this) {
        tzWarning() << "Index is for a different model";
        return false;
    }

    if (index.row() < 0) {
        tzWarning() << "Index has negative row";
        return false;
    }

    if (index.column() < 0) {
        tzWarning() << "Index has negative column";
        return false;
    }

    if (!(options & CheckIndexOption::DoNotUseParent)) {
        const TzModelIndex parentIndex = index.parent();
        if (options & CheckIndexOption::ParentIsInvalid) {
            if (parentIndex.isValid()) {
                tzWarning() << "Index has valid parent (expected an invalid parent)";
                return false;
            }
        }

        const int rc = rowCount(parentIndex);
        if (index.row() >= rc) {
            tzWarning() << "Index has out of range row";
            return false;
        }

        const int cc = columnCount(parentIndex);
        if (index.column() >= cc) {
            tzWarning() << "Index has out of range column";
            return false;
        }
    }

    return true;
}

// ── TzAbstractTableModel ─────────────────────────────────────────────────────

TzAbstractTableModel::TzAbstractTableModel()
    : TzAbstractItemModel()
{
}

TzAbstractTableModel::TzAbstractTableModel(TzAbstractItemModelPrivate &dd)
    : TzAbstractItemModel(dd)
{
}

TzAbstractTableModel::~TzAbstractTableModel()
{
}

TzModelIndex TzAbstractTableModel::index(int row, int column, const TzModelIndex &parent) const
{
    return hasIndex(row, column, parent) ? createIndex(row, column) : TzModelIndex();
}

TzModelIndex TzAbstractTableModel::parent(const TzModelIndex &) const
{
    return TzModelIndex();
}

TzModelIndex TzAbstractTableModel::sibling(int row, int column, const TzModelIndex &) const
{
    return index(row, column);
}

bool TzAbstractTableModel::hasChildren(const TzModelIndex &parent) const
{
    if (!parent.isValid())
        return rowCount(parent) > 0 && columnCount(parent) > 0;
    return false;
}

TzItemFlags TzAbstractTableModel::flags(const TzModelIndex &index) const
{
    TzItemFlags f = TzAbstractItemModel::flags(index);
    if (index.isValid())
        f |= TzItemFlag::ItemNeverHasChildren;
    return f;
}

// ── TzAbstractItemModelPrivate::Persistent ────────────────────────────────────

void TzAbstractItemModelPrivate::Persistent::insertMultiAtEnd(const TzModelIndex& key, TzPersistentModelIndexData *data)
{
    auto newIt = indexes.insert({key, data});
    auto it = newIt;
    ++it;
    while (it != indexes.end() && it->first == key) {
        std::swap(newIt->second, it->second);
        newIt = it;
        ++it;
    }
}
