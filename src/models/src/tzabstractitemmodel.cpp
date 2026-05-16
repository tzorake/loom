#include <models/tzabstractitemmodel.h>
#include <tzabstractitemmodel_p.h>
#include <stdexcept>
#include <algorithm>
#include <iostream>

TzPersistentTzModelIndexData* TzPersistentTzModelIndexData::create(const TzModelIndex& index)
{
    if (!index.isValid())
        return nullptr;
    
    TzAbstractItemModel* model = const_cast<TzAbstractItemModel*>(index.model());
    auto* d = model->d_func();
    auto& indexes = d->persistent.indexes;
    
    auto it = indexes.find(index);
    if (it != indexes.end())
        return it->second;
    
    TzPersistentTzModelIndexData* data = new TzPersistentTzModelIndexData(index);
    indexes.emplace(index, data);
    return data;
}

void TzPersistentTzModelIndexData::destroy(TzPersistentTzModelIndexData* data)
{
    if (!data)
        return;
    
    if (data->ref != 0)
        return;
    
    TzAbstractItemModel* model = const_cast<TzAbstractItemModel*>(data->index.model());
    if (model) {
        auto* d = model->d_func();
        d->removePersistentIndexData(data);
    }
    
    delete data;
}

TzPersistentTzModelIndex::TzPersistentTzModelIndex() noexcept
    : d(nullptr)
{
}

TzPersistentTzModelIndex::TzPersistentTzModelIndex(const TzPersistentTzModelIndex& other) noexcept
    : d(other.d)
{
    if (d)
        d->ref++;
}

TzPersistentTzModelIndex::TzPersistentTzModelIndex(const TzModelIndex& index)
    : d(nullptr)
{
    if (index.isValid()) {
        d = TzPersistentTzModelIndexData::create(index);
        if (d)
            d->ref++;
    }
}

TzPersistentTzModelIndex::~TzPersistentTzModelIndex()
{
    if (d) {
        d->ref--;
        if (d->ref == 0) {
            TzPersistentTzModelIndexData::destroy(d);
        }
    }
}

TzPersistentTzModelIndex& TzPersistentTzModelIndex::operator=(const TzPersistentTzModelIndex& other) noexcept
{
    if (d == other.d)
        return *this;
    
    if (d) {
        d->ref--;
        if (d->ref == 0)
            TzPersistentTzModelIndexData::destroy(d);
    }
    
    d = other.d;
    if (d)
        d->ref++;
    
    return *this;
}

TzPersistentTzModelIndex& TzPersistentTzModelIndex::operator=(const TzModelIndex& index)
{
    if (d) {
        d->ref--;
        if (d->ref == 0)
            TzPersistentTzModelIndexData::destroy(d);
        d = nullptr;
    }
    
    if (index.isValid()) {
        d = TzPersistentTzModelIndexData::create(index);
        if (d)
            d->ref++;
    }
    
    return *this;
}

TzPersistentTzModelIndex::TzPersistentTzModelIndex(TzPersistentTzModelIndex&& other) noexcept
    : d(other.d)
{
    other.d = nullptr;
}

TzPersistentTzModelIndex& TzPersistentTzModelIndex::operator=(TzPersistentTzModelIndex&& other) noexcept
{
    if (this != &other) {
        if (d) {
            d->ref--;
            if (d->ref == 0)
                TzPersistentTzModelIndexData::destroy(d);
        }
        d = other.d;
        other.d = nullptr;
    }
    return *this;
}

bool TzPersistentTzModelIndex::operator==(const TzPersistentTzModelIndex& other) const noexcept
{
    if (d && other.d)
        return d->index == other.d->index;
    return d == other.d;
}

bool TzPersistentTzModelIndex::operator<(const TzPersistentTzModelIndex& other) const noexcept
{
    if (d && other.d)
        return d->index < other.d->index;
    return d < other.d;
}

int TzPersistentTzModelIndex::row() const noexcept
{
    return d ? d->index.row() : -1;
}

int TzPersistentTzModelIndex::column() const noexcept
{
    return d ? d->index.column() : -1;
}

void* TzPersistentTzModelIndex::internalPointer() const noexcept
{
    return d ? d->index.internalPointer() : nullptr;
}

TzModelIndexId TzPersistentTzModelIndex::internalId() const noexcept
{
    return d ? d->index.internalId() : 0;
}

const TzAbstractItemModel* TzPersistentTzModelIndex::model() const noexcept
{
    return d ? d->index.model() : nullptr;
}

bool TzPersistentTzModelIndex::isValid() const noexcept
{
    return d && d->index.isValid();
}

TzPersistentTzModelIndex::operator TzModelIndex() const
{
    return d ? d->index : TzModelIndex();
}

TzAbstractItemModelPrivate::TzAbstractItemModelPrivate()
{
}

TzAbstractItemModelPrivate::~TzAbstractItemModelPrivate()
{
}

void TzAbstractItemModelPrivate::removePersistentIndexData(TzPersistentTzModelIndexData* data)
{
    if (!data)
        return;
    
    if (data->index.isValid()) {
        // Remove from hash (Qt uses remove which removes one entry)
        auto range = persistent.indexes.equal_range(data->index);
        for (auto it = range.first; it != range.second; ++it) {
            if (it->second == data) {
                persistent.indexes.erase(it);
                break;
            }
        }
    }
    
    // Clean up from moved/invalidated lists (Qt does this)
    for (auto& movedList : persistent.moved) {
        movedList.erase(std::remove(movedList.begin(), movedList.end(), data), movedList.end());
    }
    
    for (auto& invalidatedList : persistent.invalidated) {
        invalidatedList.erase(std::remove(invalidatedList.begin(), invalidatedList.end(), data), 
                             invalidatedList.end());
    }
}

void TzAbstractItemModelPrivate::invalidatePersistentIndexes()
{
    // Qt clears all persistent indexes
    for (auto& pair : persistent.indexes) {
        pair.second->index = TzModelIndex();
    }
    persistent.indexes.clear();
}

void TzAbstractItemModelPrivate::invalidatePersistentIndex(const TzModelIndex& index)
{
    auto range = persistent.indexes.equal_range(index);
    for (auto it = range.first; it != range.second; ++it) {
        it->second->index = TzModelIndex();
    }
    persistent.indexes.erase(index);
}

bool TzAbstractItemModel::beginInsertRows(const TzModelIndex& parent, int first, int last)
{
    if (first < 0 || last < first)
        return false;
    
    auto* d = d_func();
    d->insertRowsChange = TzAbstractItemModelPrivate::Change(parent, first, last);
    d->rowsAboutToBeInserted(parent, first, last);
    m_events.emit("rowsAboutToBeInserted", parent, first, last);
    return true;
}

void TzAbstractItemModel::endInsertRows()
{
    auto* d = d_func();
    d->rowsInserted(d->insertRowsChange.parent, d->insertRowsChange.first, d->insertRowsChange.last);
    m_events.emit("rowsInserted", d->insertRowsChange.parent, 
                 d->insertRowsChange.first, d->insertRowsChange.last);
}

bool TzAbstractItemModel::beginRemoveRows(const TzModelIndex& parent, int first, int last)
{
    if (first < 0 || last < first || last >= rowCount(parent))
        return false;
    
    auto* d = d_func();
    d->removeRowsChange = TzAbstractItemModelPrivate::Change(parent, first, last);
    d->rowsAboutToBeRemoved(parent, first, last);
    m_events.emit("rowsAboutToBeRemoved", parent, first, last);
    return true;
}

void TzAbstractItemModel::endRemoveRows()
{
    auto* d = d_func();
    d->rowsRemoved(d->removeRowsChange.parent, d->removeRowsChange.first, d->removeRowsChange.last);
    m_events.emit("rowsRemoved", d->removeRowsChange.parent,
                 d->removeRowsChange.first, d->removeRowsChange.last);
}

bool TzAbstractItemModel::beginMoveRows(const TzModelIndex& sourceParent, int sourceFirst, int sourceLast,
                                     const TzModelIndex& destinationParent, int destinationRow)
{
    auto* d = d_func();
    if (!d->allowMove(sourceParent, sourceFirst, sourceLast, destinationParent, destinationRow, Orientation::Vertical))
        return false;
    
    d->itemsAboutToBeMoved(sourceParent, sourceFirst, sourceLast, destinationParent, destinationRow, Orientation::Vertical);
    m_events.emit("rowsAboutToBeMoved", sourceParent, sourceFirst, sourceLast, destinationParent, destinationRow);
    return true;
}

void TzAbstractItemModel::endMoveRows()
{
    // Qt calls itemsMoved which handles persistent index updates
    auto* d = d_func();
    // The source/dest info is stored during itemsAboutToBeMoved
    m_events.emit("rowsMoved");
}

bool TzAbstractItemModel::beginInsertColumns(const TzModelIndex& parent, int first, int last)
{
    if (first < 0 || last < first)
        return false;
    
    auto* d = d_func();
    d->insertColumnsChange = TzAbstractItemModelPrivate::Change(parent, first, last);
    d->columnsAboutToBeInserted(parent, first, last);
    m_events.emit("columnsAboutToBeInserted", parent, first, last);
    return true;
}

void TzAbstractItemModel::endInsertColumns()
{
    auto* d = d_func();
    d->columnsInserted(d->insertColumnsChange.parent, d->insertColumnsChange.first, d->insertColumnsChange.last);
    m_events.emit("columnsInserted", d->insertColumnsChange.parent,
                 d->insertColumnsChange.first, d->insertColumnsChange.last);
}

bool TzAbstractItemModel::beginRemoveColumns(const TzModelIndex& parent, int first, int last)
{
    if (first < 0 || last < first || last >= columnCount(parent))
        return false;
    
    auto* d = d_func();
    d->removeColumnsChange = TzAbstractItemModelPrivate::Change(parent, first, last);
    d->columnsAboutToBeRemoved(parent, first, last);
    m_events.emit("columnsAboutToBeRemoved", parent, first, last);
    return true;
}

void TzAbstractItemModel::endRemoveColumns()
{
    auto* d = d_func();
    d->columnsRemoved(d->removeColumnsChange.parent, d->removeColumnsChange.first, d->removeColumnsChange.last);
    m_events.emit("columnsRemoved", d->removeColumnsChange.parent,
                 d->removeColumnsChange.first, d->removeColumnsChange.last);
}

bool TzAbstractItemModel::beginMoveColumns(const TzModelIndex& sourceParent, int sourceFirst, int sourceLast,
                                        const TzModelIndex& destinationParent, int destinationColumn)
{
    auto* d = d_func();
    if (!d->allowMove(sourceParent, sourceFirst, sourceLast, destinationParent, destinationColumn, Orientation::Horizontal))
        return false;
    
    d->itemsAboutToBeMoved(sourceParent, sourceFirst, sourceLast, destinationParent, destinationColumn, Orientation::Horizontal);
    m_events.emit("columnsAboutToBeMoved", sourceParent, sourceFirst, sourceLast, destinationParent, destinationColumn);
    return true;
}

void TzAbstractItemModel::endMoveColumns()
{
    m_events.emit("columnsMoved");
}

// Model reset

void TzAbstractItemModel::beginResetModel()
{
    m_events.emit("modelAboutToBeReset");
}

void TzAbstractItemModel::endResetModel()
{
    auto* d = d_func();
    d->invalidatePersistentIndexes();
    m_events.emit("modelReset");
}

void TzAbstractItemModelPrivate::rowsAboutToBeInserted(const TzModelIndex& parent, int first, int last)
{
    (void)last;
    std::vector<TzPersistentTzModelIndexData*> persistent_moved;
    
    auto* q = q_func();
    if (first < q->rowCount(parent)) {
        for (const auto& pair : persistent.indexes) {
            TzPersistentTzModelIndexData* data = pair.second;
            const TzModelIndex& index = data->index;
            if (index.row() >= first && index.isValid() && index.parent() == parent) {
                persistent_moved.push_back(data);
            }
        }
    }
    
    persistent.moved.push_back(persistent_moved);
}

void TzAbstractItemModelPrivate::rowsInserted(const TzModelIndex& parent, int first, int last)
{
    std::vector<TzPersistentTzModelIndexData*> persistent_moved = persistent.moved.back();
    persistent.moved.pop_back();
    
    const int count = (last - first) + 1;
    auto* q = q_func();
    
    for (auto* data : persistent_moved) {
        TzModelIndex old = data->index;
        
        // Remove from hash
        auto range = persistent.indexes.equal_range(old);
        for (auto it = range.first; it != range.second; ++it) {
            if (it->second == data) {
                persistent.indexes.erase(it);
                break;
            }
        }
        
        // Update index
        data->index = q->index(old.row() + count, old.column(), parent);
        if (data->index.isValid()) {
            persistent.insertMultiAtEnd(data->index, data);
        }
    }
}

void TzAbstractItemModelPrivate::rowsAboutToBeRemoved(const TzModelIndex& parent, int first, int last)
{
    std::vector<TzPersistentTzModelIndexData*> persistent_moved;
    std::vector<TzPersistentTzModelIndexData*> persistent_invalidated;
    
    for (const auto& pair : persistent.indexes) {
        TzPersistentTzModelIndexData* data = pair.second;
        const TzModelIndex& index = data->index;
        
        if (index.isValid() && index.parent() == parent) {
            if (index.row() >= first && index.row() <= last) {
                persistent_invalidated.push_back(data);
            } else if (index.row() > last) {
                persistent_moved.push_back(data);
            }
        }
    }
    
    persistent.moved.push_back(persistent_moved);
    persistent.invalidated.push_back(persistent_invalidated);
}

void TzAbstractItemModelPrivate::rowsRemoved(const TzModelIndex& parent, int first, int last)
{
    std::vector<TzPersistentTzModelIndexData*> persistent_moved = persistent.moved.back();
    persistent.moved.pop_back();
    
    std::vector<TzPersistentTzModelIndexData*> persistent_invalidated = persistent.invalidated.back();
    persistent.invalidated.pop_back();
    
    const int count = (last - first) + 1;
    auto* q = q_func();
    
    // Invalidate removed indices
    for (auto* data : persistent_invalidated) {
        auto range = persistent.indexes.equal_range(data->index);
        for (auto it = range.first; it != range.second; ++it) {
            if (it->second == data) {
                persistent.indexes.erase(it);
                break;
            }
        }
        data->index = TzModelIndex();
    }
    
    // Shift remaining indices
    for (auto* data : persistent_moved) {
        TzModelIndex old = data->index;
        
        // Remove from hash
        auto range = persistent.indexes.equal_range(old);
        for (auto it = range.first; it != range.second; ++it) {
            if (it->second == data) {
                persistent.indexes.erase(it);
                break;
            }
        }
        
        // Update index
        data->index = q->index(old.row() - count, old.column(), parent);
        if (data->index.isValid()) {
            persistent.insertMultiAtEnd(data->index, data);
        }
    }
}

// Column operations (similar pattern)

void TzAbstractItemModelPrivate::columnsAboutToBeInserted(const TzModelIndex& parent, int first, int last)
{
    (void)last;
    std::vector<TzPersistentTzModelIndexData*> persistent_moved;
    
    auto* q = q_func();
    if (first < q->columnCount(parent)) {
        for (const auto& pair : persistent.indexes) {
            TzPersistentTzModelIndexData* data = pair.second;
            const TzModelIndex& index = data->index;
            if (index.column() >= first && index.isValid() && index.parent() == parent) {
                persistent_moved.push_back(data);
            }
        }
    }
    
    persistent.moved.push_back(persistent_moved);
}

void TzAbstractItemModelPrivate::columnsInserted(const TzModelIndex& parent, int first, int last)
{
    std::vector<TzPersistentTzModelIndexData*> persistent_moved = persistent.moved.back();
    persistent.moved.pop_back();
    
    const int count = (last - first) + 1;
    auto* q = q_func();
    
    for (auto* data : persistent_moved) {
        TzModelIndex old = data->index;
        
        auto range = persistent.indexes.equal_range(old);
        for (auto it = range.first; it != range.second; ++it) {
            if (it->second == data) {
                persistent.indexes.erase(it);
                break;
            }
        }
        
        data->index = q->index(old.row(), old.column() + count, parent);
        if (data->index.isValid()) {
            persistent.insertMultiAtEnd(data->index, data);
        }
    }
}

void TzAbstractItemModelPrivate::columnsAboutToBeRemoved(const TzModelIndex& parent, int first, int last)
{
    std::vector<TzPersistentTzModelIndexData*> persistent_moved;
    std::vector<TzPersistentTzModelIndexData*> persistent_invalidated;
    
    for (const auto& pair : persistent.indexes) {
        TzPersistentTzModelIndexData* data = pair.second;
        const TzModelIndex& index = data->index;
        
        if (index.isValid() && index.parent() == parent) {
            if (index.column() >= first && index.column() <= last) {
                persistent_invalidated.push_back(data);
            } else if (index.column() > last) {
                persistent_moved.push_back(data);
            }
        }
    }
    
    persistent.moved.push_back(persistent_moved);
    persistent.invalidated.push_back(persistent_invalidated);
}

void TzAbstractItemModelPrivate::columnsRemoved(const TzModelIndex& parent, int first, int last)
{
    std::vector<TzPersistentTzModelIndexData*> persistent_moved = persistent.moved.back();
    persistent.moved.pop_back();
    
    std::vector<TzPersistentTzModelIndexData*> persistent_invalidated = persistent.invalidated.back();
    persistent.invalidated.pop_back();
    
    const int count = (last - first) + 1;
    auto* q = q_func();
    
    for (auto* data : persistent_invalidated) {
        auto range = persistent.indexes.equal_range(data->index);
        for (auto it = range.first; it != range.second; ++it) {
            if (it->second == data) {
                persistent.indexes.erase(it);
                break;
            }
        }
        data->index = TzModelIndex();
    }
    
    for (auto* data : persistent_moved) {
        TzModelIndex old = data->index;
        
        auto range = persistent.indexes.equal_range(old);
        for (auto it = range.first; it != range.second; ++it) {
            if (it->second == data) {
                persistent.indexes.erase(it);
                break;
            }
        }
        
        data->index = q->index(old.row(), old.column() - count, parent);
        if (data->index.isValid()) {
            persistent.insertMultiAtEnd(data->index, data);
        }
    }
}

#define Q_UNUSED(x) (void)x

// Move validation (Qt-accurate)
bool TzAbstractItemModelPrivate::allowMove(const TzModelIndex& srcParent, int start, int end,
                                        const TzModelIndex& destinationParent, int destinationStart,
                                        TzAbstractItemModel::Orientation orientation)
{
    // Qt's validation logic
    if (start < 0 || start > end)
        return false;
    
    auto* q = q_func();
    int count = (orientation == TzAbstractItemModel::Orientation::Vertical) ? 
                q->rowCount(srcParent) : q->columnCount(srcParent);
    
    if (end >= count)
        return false;
    
    if (destinationStart < 0)
        return false;
    
    int destCount = (orientation == TzAbstractItemModel::Orientation::Vertical) ?
                    q->rowCount(destinationParent) : q->columnCount(destinationParent);
    
    if (destinationStart > destCount)
        return false;
    
    // Can't move to within itself
    if (srcParent == destinationParent) {
        if (destinationStart >= start && destinationStart <= end + 1)
            return false;
    }
    
    return true;
}

void TzAbstractItemModelPrivate::itemsAboutToBeMoved(const TzModelIndex& sourceParent, int sourceFirst, int sourceLast,
                                                  const TzModelIndex& destinationParent, int destinationChild,
                                                  TzAbstractItemModel::Orientation orientation)
{
    // This is complex in Qt - simplified version for now
    // Would need full implementation for move operations
    Q_UNUSED(sourceParent);
    Q_UNUSED(sourceFirst);
    Q_UNUSED(sourceLast);
    Q_UNUSED(destinationParent);
    Q_UNUSED(destinationChild);
    Q_UNUSED(orientation);
}

void TzAbstractItemModelPrivate::itemsMoved(const TzModelIndex& sourceParent, int sourceFirst, int sourceLast,
                                         const TzModelIndex& destinationParent, int destinationChild,
                                         TzAbstractItemModel::Orientation orientation)
{
    // Complex persistent index updates for moves
    Q_UNUSED(sourceParent);
    Q_UNUSED(sourceFirst);
    Q_UNUSED(sourceLast);
    Q_UNUSED(destinationParent);
    Q_UNUSED(destinationChild);
    Q_UNUSED(orientation);
}

void TzAbstractItemModelPrivate::movePersistentIndexes(const std::vector<TzPersistentTzModelIndexData*>& indexes,
                                                     int change, const TzModelIndex& parent,
                                                     TzAbstractItemModel::Orientation orientation)
{
    Q_UNUSED(indexes);
    Q_UNUSED(change);
    Q_UNUSED(parent);
    Q_UNUSED(orientation);
}

TzAbstractItemModel::TzAbstractItemModel()
    : d_ptr(new TzAbstractItemModelPrivate)
{
    d_ptr->q_ptr = this;
}

TzAbstractItemModel::~TzAbstractItemModel()
{
}

bool TzAbstractItemModel::setData(const TzModelIndex& index, const std::any& value, int role)
{
    (void)index; (void)value; (void)role;
    return false;
}

std::any TzAbstractItemModel::headerData(int section, Orientation orientation, int role) const
{
    (void)role;
    if (orientation == Orientation::Horizontal)
        return std::string("Column ") + std::to_string(section);
    return std::string("Row ") + std::to_string(section);
}

bool TzAbstractItemModel::setHeaderData(int section, Orientation orientation, const std::any& value, int role)
{
    (void)section; (void)orientation; (void)value; (void)role;
    return false;
}

TzItemFlag TzAbstractItemModel::flags(const TzModelIndex& index) const
{
    if (!index.isValid())
        return TzItemFlag::NoItemFlags;
    return TzItemFlag::IsSelectable | TzItemFlag::IsEnabled;
}

bool TzAbstractItemModel::insertRows(int row, int count, const TzModelIndex& parent)
{
    (void)row; (void)count; (void)parent;
    return false;
}

bool TzAbstractItemModel::insertColumns(int column, int count, const TzModelIndex& parent)
{
    (void)column; (void)count; (void)parent;
    return false;
}

bool TzAbstractItemModel::removeRows(int row, int count, const TzModelIndex& parent)
{
    (void)row; (void)count; (void)parent;
    return false;
}

bool TzAbstractItemModel::removeColumns(int column, int count, const TzModelIndex& parent)
{
    (void)column; (void)count; (void)parent;
    return false;
}

bool TzAbstractItemModel::moveRows(const TzModelIndex& sourceParent, int sourceRow, int count,
                                const TzModelIndex& destinationParent, int destinationChild)
{
    (void)sourceParent; (void)sourceRow; (void)count;
    (void)destinationParent; (void)destinationChild;
    return false;
}

bool TzAbstractItemModel::moveColumns(const TzModelIndex& sourceParent, int sourceColumn, int count,
                                   const TzModelIndex& destinationParent, int destinationChild)
{
    (void)sourceParent; (void)sourceColumn; (void)count;
    (void)destinationParent; (void)destinationChild;
    return false;
}

bool TzAbstractItemModel::hasIndex(int row, int column, const TzModelIndex& parent) const
{
    if (row < 0 || column < 0)
        return false;
    return row < rowCount(parent) && column < columnCount(parent);
}

TzModelIndex TzAbstractItemModel::sibling(int row, int column, const TzModelIndex& index) const
{
    return this->index(row, column, parent(index));
}

bool TzAbstractItemModel::hasChildren(const TzModelIndex& parent) const
{
    return rowCount(parent) > 0 && columnCount(parent) > 0;
}

void TzAbstractItemModel::sort(int column, bool ascending)
{
    (void)column; (void)ascending;
}

std::vector<std::string> TzAbstractItemModel::mimeTypes() const
{
    return {};
}

std::any TzAbstractItemModel::mimeData(const std::vector<TzModelIndex>& indexes) const
{
    (void)indexes;
    return std::any();
}

bool TzAbstractItemModel::canDropMimeData(const std::any& data, int action, int row, int column, const TzModelIndex& parent) const
{
    (void)data; (void)action; (void)row; (void)column; (void)parent;
    return false;
}

bool TzAbstractItemModel::dropMimeData(const std::any& data, int action, int row, int column, const TzModelIndex& parent)
{
    (void)data; (void)action; (void)row; (void)column; (void)parent;
    return false;
}

TzAbstractItemModel::TzAbstractItemModel(TzAbstractItemModelPrivate& dd)
    : d_ptr(&dd)
{
    d_ptr->q_ptr = this;
}

TzModelIndex TzAbstractItemModel::createIndex(int row, int column, void* ptr) const
{
    return TzModelIndex(row, column, ptr, this);
}

TzModelIndex TzAbstractItemModel::createIndex(int row, int column, TzModelIndexId id) const
{
    return TzModelIndex(row, column, reinterpret_cast<void*>(id), this);
}

void TzAbstractItemModel::emitDataChanged(const TzModelIndex& topLeft, const TzModelIndex& bottomRight,
                                       const std::vector<int>& roles)
{
    m_events.emit("dataChanged", topLeft, bottomRight, roles);
}

void TzAbstractItemModel::emitHeaderDataChanged(Orientation orientation, int first, int last)
{
    m_events.emit("headerDataChanged", static_cast<int>(orientation), first, last);
}

void TzAbstractItemModel::changePersistentIndex(const TzModelIndex& from, const TzModelIndex& to)
{
    auto* d = d_func();
    
    auto range = d->persistent.indexes.equal_range(from);
    std::vector<TzPersistentTzModelIndexData*> datas;
    for (auto it = range.first; it != range.second; ++it) {
        datas.push_back(it->second);
    }
    d->persistent.indexes.erase(from);
    
    for (auto* data : datas) {
        data->index = to;
        if (to.isValid()) {
            d->persistent.insertMultiAtEnd(to, data);
        }
    }
}

void TzAbstractItemModel::changePersistentIndexList(const std::vector<TzModelIndex>& from,
                                                  const std::vector<TzModelIndex>& to)
{
    if (from.size() != to.size()) {
        throw std::invalid_argument("from and to lists must have same size");
    }
    
    for (size_t i = 0; i < from.size(); ++i) {
        changePersistentIndex(from[i], to[i]);
    }
}

std::vector<TzModelIndex> TzAbstractItemModel::persistentIndexList() const
{
    auto* d = d_func();
    std::vector<TzModelIndex> result;
    result.reserve(d->persistent.indexes.size());
    
    for (const auto& pair : d->persistent.indexes) {
        if (pair.second->index.isValid()) {
            result.push_back(pair.second->index);
        }
    }
    
    return result;
}
