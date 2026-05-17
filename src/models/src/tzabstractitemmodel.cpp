// #include <loom/tzabstractitemmodel.hpp>
// #include <algorithm>
// #include <iostream>
// #include <stdexcept>
// #include <tzabstractitemmodel_p.hpp>

// TzPersistentTzModelIndexData *TzPersistentTzModelIndexData::create(const TzModelIndex &index)
// {
//     if (!index.isValid())
//         return nullptr;

//     TzAbstractItemModel *model = const_cast<TzAbstractItemModel *>(index.model());
//     auto *d = model->d_func();
//     auto &indexes = d->persistent.indexes;

//     auto it = indexes.find(index);
//     if (it != indexes.end())
//         return it->second;

//     TzPersistentTzModelIndexData *data = new TzPersistentTzModelIndexData(index);
//     indexes.emplace(index, data);
//     return data;
// }

// void TzPersistentTzModelIndexData::destroy(TzPersistentTzModelIndexData *data)
// {
//     if (!data)
//         return;

//     if (data->ref != 0)
//         return;

//     TzAbstractItemModel *model = const_cast<TzAbstractItemModel *>(data->index.model());
//     if (model) {
//         auto *d = model->d_func();
//         d->removePersistentIndexData(data);
//     }

//     delete data;
// }

// TzPersistentTzModelIndex::TzPersistentTzModelIndex() noexcept
//     : d(nullptr)
// {}

// TzPersistentTzModelIndex::TzPersistentTzModelIndex(const TzPersistentTzModelIndex &other) noexcept
//     : d(other.d)
// {
//     if (d)
//         d->ref++;
// }

// TzPersistentTzModelIndex::TzPersistentTzModelIndex(const TzModelIndex &index)
//     : d(nullptr)
// {
//     if (index.isValid()) {
//         d = TzPersistentTzModelIndexData::create(index);
//         if (d)
//             d->ref++;
//     }
// }

// TzPersistentTzModelIndex::~TzPersistentTzModelIndex()
// {
//     if (d) {
//         d->ref--;
//         if (d->ref == 0) {
//             TzPersistentTzModelIndexData::destroy(d);
//         }
//     }
// }

// TzPersistentTzModelIndex &TzPersistentTzModelIndex::operator=(
//     const TzPersistentTzModelIndex &other) noexcept
// {
//     if (d == other.d)
//         return *this;

//     if (d) {
//         d->ref--;
//         if (d->ref == 0)
//             TzPersistentTzModelIndexData::destroy(d);
//     }

//     d = other.d;
//     if (d)
//         d->ref++;

//     return *this;
// }

// TzPersistentTzModelIndex &TzPersistentTzModelIndex::operator=(const TzModelIndex &index)
// {
//     if (d) {
//         d->ref--;
//         if (d->ref == 0)
//             TzPersistentTzModelIndexData::destroy(d);
//         d = nullptr;
//     }

//     if (index.isValid()) {
//         d = TzPersistentTzModelIndexData::create(index);
//         if (d)
//             d->ref++;
//     }

//     return *this;
// }

// TzPersistentTzModelIndex::TzPersistentTzModelIndex(TzPersistentTzModelIndex &&other) noexcept
//     : d(other.d)
// {
//     other.d = nullptr;
// }

// TzPersistentTzModelIndex &TzPersistentTzModelIndex::operator=(
//     TzPersistentTzModelIndex &&other) noexcept
// {
//     if (this != &other) {
//         if (d) {
//             d->ref--;
//             if (d->ref == 0)
//                 TzPersistentTzModelIndexData::destroy(d);
//         }
//         d = other.d;
//         other.d = nullptr;
//     }
//     return *this;
// }

// bool TzPersistentTzModelIndex::operator==(const TzPersistentTzModelIndex &other) const noexcept
// {
//     if (d && other.d)
//         return d->index == other.d->index;
//     return d == other.d;
// }

// bool TzPersistentTzModelIndex::operator<(const TzPersistentTzModelIndex &other) const noexcept
// {
//     if (d && other.d)
//         return d->index < other.d->index;
//     return d < other.d;
// }

// int TzPersistentTzModelIndex::row() const noexcept
// {
//     return d ? d->index.row() : -1;
// }

// int TzPersistentTzModelIndex::column() const noexcept
// {
//     return d ? d->index.column() : -1;
// }

// void *TzPersistentTzModelIndex::internalPointer() const noexcept
// {
//     return d ? d->index.internalPointer() : nullptr;
// }

// TzModelIndexId TzPersistentTzModelIndex::internalId() const noexcept
// {
//     return d ? d->index.internalId() : 0;
// }

// const TzAbstractItemModel *TzPersistentTzModelIndex::model() const noexcept
// {
//     return d ? d->index.model() : nullptr;
// }

// bool TzPersistentTzModelIndex::isValid() const noexcept
// {
//     return d && d->index.isValid();
// }

// TzPersistentTzModelIndex::operator TzModelIndex() const
// {
//     return d ? d->index : TzModelIndex();
// }

// TzAbstractItemModelPrivate::TzAbstractItemModelPrivate() {}

// TzAbstractItemModelPrivate::~TzAbstractItemModelPrivate() {}

// void TzAbstractItemModelPrivate::removePersistentIndexData(TzPersistentTzModelIndexData *data)
// {
//     if (!data)
//         return;

//     if (data->index.isValid()) {
//         // Remove from hash (Qt uses remove which removes one entry)
//         auto range = persistent.indexes.equal_range(data->index);
//         for (auto it = range.first; it != range.second; ++it) {
//             if (it->second == data) {
//                 persistent.indexes.erase(it);
//                 break;
//             }
//         }
//     }

//     // Clean up from moved/invalidated lists (Qt does this)
//     for (auto &movedList : persistent.moved) {
//         movedList.erase(std::remove(movedList.begin(), movedList.end(), data), movedList.end());
//     }

//     for (auto &invalidatedList : persistent.invalidated) {
//         invalidatedList.erase(std::remove(invalidatedList.begin(), invalidatedList.end(), data),
//                               invalidatedList.end());
//     }
// }

// void TzAbstractItemModelPrivate::invalidatePersistentIndexes()
// {
//     // Qt clears all persistent indexes
//     for (auto &pair : persistent.indexes) {
//         pair.second->index = TzModelIndex();
//     }
//     persistent.indexes.clear();
// }

// void TzAbstractItemModelPrivate::invalidatePersistentIndex(const TzModelIndex &index)
// {
//     auto range = persistent.indexes.equal_range(index);
//     for (auto it = range.first; it != range.second; ++it) {
//         it->second->index = TzModelIndex();
//     }
//     persistent.indexes.erase(index);
// }

// bool TzAbstractItemModel::beginInsertRows(const TzModelIndex &parent, int first, int last)
// {
//     if (first < 0 || last < first)
//         return false;

//     auto *d = d_func();
//     d->insertRowsChange = TzAbstractItemModelPrivate::Change(parent, first, last);
//     d->rowsAboutToBeInserted(parent, first, last);
//     m_events.emit("rowsAboutToBeInserted", parent, first, last);
//     return true;
// }

// void TzAbstractItemModel::endInsertRows()
// {
//     auto *d = d_func();
//     d->rowsInserted(d->insertRowsChange.parent, d->insertRowsChange.first, d->insertRowsChange.last);
//     m_events.emit("rowsInserted", d->insertRowsChange.parent, d->insertRowsChange.first,
//                   d->insertRowsChange.last);
// }

// bool TzAbstractItemModel::beginRemoveRows(const TzModelIndex &parent, int first, int last)
// {
//     if (first < 0 || last < first || last >= rowCount(parent))
//         return false;

//     auto *d = d_func();
//     d->removeRowsChange = TzAbstractItemModelPrivate::Change(parent, first, last);
//     d->rowsAboutToBeRemoved(parent, first, last);
//     m_events.emit("rowsAboutToBeRemoved", parent, first, last);
//     return true;
// }

// void TzAbstractItemModel::endRemoveRows()
// {
//     auto *d = d_func();
//     d->rowsRemoved(d->removeRowsChange.parent, d->removeRowsChange.first, d->removeRowsChange.last);
//     m_events.emit("rowsRemoved", d->removeRowsChange.parent, d->removeRowsChange.first,
//                   d->removeRowsChange.last);
// }

// bool TzAbstractItemModel::beginMoveRows(const TzModelIndex &sourceParent, int sourceFirst,
//                                         int sourceLast, const TzModelIndex &destinationParent,
//                                         int destinationRow)
// {
//     auto *d = d_func();
//     if (!d->allowMove(sourceParent, sourceFirst, sourceLast, destinationParent, destinationRow,
//                       Orientation::Vertical))
//         return false;

//     d->itemsAboutToBeMoved(sourceParent, sourceFirst, sourceLast, destinationParent, destinationRow,
//                            Orientation::Vertical);
//     m_events.emit("rowsAboutToBeMoved", sourceParent, sourceFirst, sourceLast, destinationParent,
//                   destinationRow);
//     return true;
// }

// void TzAbstractItemModel::endMoveRows()
// {
//     // Qt calls itemsMoved which handles persistent index updates
//     auto *d = d_func();
//     // The source/dest info is stored during itemsAboutToBeMoved
//     m_events.emit("rowsMoved");
// }

// bool TzAbstractItemModel::beginInsertColumns(const TzModelIndex &parent, int first, int last)
// {
//     if (first < 0 || last < first)
//         return false;

//     auto *d = d_func();
//     d->insertColumnsChange = TzAbstractItemModelPrivate::Change(parent, first, last);
//     d->columnsAboutToBeInserted(parent, first, last);
//     m_events.emit("columnsAboutToBeInserted", parent, first, last);
//     return true;
// }

// void TzAbstractItemModel::endInsertColumns()
// {
//     auto *d = d_func();
//     d->columnsInserted(d->insertColumnsChange.parent, d->insertColumnsChange.first,
//                        d->insertColumnsChange.last);
//     m_events.emit("columnsInserted", d->insertColumnsChange.parent, d->insertColumnsChange.first,
//                   d->insertColumnsChange.last);
// }

// bool TzAbstractItemModel::beginRemoveColumns(const TzModelIndex &parent, int first, int last)
// {
//     if (first < 0 || last < first || last >= columnCount(parent))
//         return false;

//     auto *d = d_func();
//     d->removeColumnsChange = TzAbstractItemModelPrivate::Change(parent, first, last);
//     d->columnsAboutToBeRemoved(parent, first, last);
//     m_events.emit("columnsAboutToBeRemoved", parent, first, last);
//     return true;
// }

// void TzAbstractItemModel::endRemoveColumns()
// {
//     auto *d = d_func();
//     d->columnsRemoved(d->removeColumnsChange.parent, d->removeColumnsChange.first,
//                       d->removeColumnsChange.last);
//     m_events.emit("columnsRemoved", d->removeColumnsChange.parent, d->removeColumnsChange.first,
//                   d->removeColumnsChange.last);
// }

// bool TzAbstractItemModel::beginMoveColumns(const TzModelIndex &sourceParent, int sourceFirst,
//                                            int sourceLast, const TzModelIndex &destinationParent,
//                                            int destinationColumn)
// {
//     auto *d = d_func();
//     if (!d->allowMove(sourceParent, sourceFirst, sourceLast, destinationParent, destinationColumn,
//                       Orientation::Horizontal))
//         return false;

//     d->itemsAboutToBeMoved(sourceParent, sourceFirst, sourceLast, destinationParent,
//                            destinationColumn, Orientation::Horizontal);
//     m_events.emit("columnsAboutToBeMoved", sourceParent, sourceFirst, sourceLast, destinationParent,
//                   destinationColumn);
//     return true;
// }

// void TzAbstractItemModel::endMoveColumns()
// {
//     m_events.emit("columnsMoved");
// }

// // Model reset

// void TzAbstractItemModel::beginResetModel()
// {
//     m_events.emit("modelAboutToBeReset");
// }

// void TzAbstractItemModel::endResetModel()
// {
//     auto *d = d_func();
//     d->invalidatePersistentIndexes();
//     m_events.emit("modelReset");
// }

// void TzAbstractItemModelPrivate::rowsAboutToBeInserted(const TzModelIndex &parent, int first,
//                                                        int last)
// {
//     (void) last;
//     std::vector<TzPersistentTzModelIndexData *> persistent_moved;

//     auto *q = q_func();
//     if (first < q->rowCount(parent)) {
//         for (const auto &pair : persistent.indexes) {
//             TzPersistentTzModelIndexData *data = pair.second;
//             const TzModelIndex &index = data->index;
//             if (index.row() >= first && index.isValid() && index.parent() == parent) {
//                 persistent_moved.push_back(data);
//             }
//         }
//     }

//     persistent.moved.push_back(persistent_moved);
// }

// void TzAbstractItemModelPrivate::rowsInserted(const TzModelIndex &parent, int first, int last)
// {
//     std::vector<TzPersistentTzModelIndexData *> persistent_moved = persistent.moved.back();
//     persistent.moved.pop_back();

//     const int count = (last - first) + 1;
//     auto *q = q_func();

//     for (auto *data : persistent_moved) {
//         TzModelIndex old = data->index;

//         // Remove from hash
//         auto range = persistent.indexes.equal_range(old);
//         for (auto it = range.first; it != range.second; ++it) {
//             if (it->second == data) {
//                 persistent.indexes.erase(it);
//                 break;
//             }
//         }

//         // Update index
//         data->index = q->index(old.row() + count, old.column(), parent);
//         if (data->index.isValid()) {
//             persistent.insertMultiAtEnd(data->index, data);
//         }
//     }
// }

// void TzAbstractItemModelPrivate::rowsAboutToBeRemoved(const TzModelIndex &parent, int first,
//                                                       int last)
// {
//     std::vector<TzPersistentTzModelIndexData *> persistent_moved;
//     std::vector<TzPersistentTzModelIndexData *> persistent_invalidated;

//     for (const auto &pair : persistent.indexes) {
//         TzPersistentTzModelIndexData *data = pair.second;
//         const TzModelIndex &index = data->index;

//         if (index.isValid() && index.parent() == parent) {
//             if (index.row() >= first && index.row() <= last) {
//                 persistent_invalidated.push_back(data);
//             } else if (index.row() > last) {
//                 persistent_moved.push_back(data);
//             }
//         }
//     }

//     persistent.moved.push_back(persistent_moved);
//     persistent.invalidated.push_back(persistent_invalidated);
// }

// void TzAbstractItemModelPrivate::rowsRemoved(const TzModelIndex &parent, int first, int last)
// {
//     std::vector<TzPersistentTzModelIndexData *> persistent_moved = persistent.moved.back();
//     persistent.moved.pop_back();

//     std::vector<TzPersistentTzModelIndexData *> persistent_invalidated
//         = persistent.invalidated.back();
//     persistent.invalidated.pop_back();

//     const int count = (last - first) + 1;
//     auto *q = q_func();

//     // Invalidate removed indices
//     for (auto *data : persistent_invalidated) {
//         auto range = persistent.indexes.equal_range(data->index);
//         for (auto it = range.first; it != range.second; ++it) {
//             if (it->second == data) {
//                 persistent.indexes.erase(it);
//                 break;
//             }
//         }
//         data->index = TzModelIndex();
//     }

//     // Shift remaining indices
//     for (auto *data : persistent_moved) {
//         TzModelIndex old = data->index;

//         // Remove from hash
//         auto range = persistent.indexes.equal_range(old);
//         for (auto it = range.first; it != range.second; ++it) {
//             if (it->second == data) {
//                 persistent.indexes.erase(it);
//                 break;
//             }
//         }

//         // Update index
//         data->index = q->index(old.row() - count, old.column(), parent);
//         if (data->index.isValid()) {
//             persistent.insertMultiAtEnd(data->index, data);
//         }
//     }
// }

// // Column operations (similar pattern)

// void TzAbstractItemModelPrivate::columnsAboutToBeInserted(const TzModelIndex &parent, int first,
//                                                           int last)
// {
//     (void) last;
//     std::vector<TzPersistentTzModelIndexData *> persistent_moved;

//     auto *q = q_func();
//     if (first < q->columnCount(parent)) {
//         for (const auto &pair : persistent.indexes) {
//             TzPersistentTzModelIndexData *data = pair.second;
//             const TzModelIndex &index = data->index;
//             if (index.column() >= first && index.isValid() && index.parent() == parent) {
//                 persistent_moved.push_back(data);
//             }
//         }
//     }

//     persistent.moved.push_back(persistent_moved);
// }

// void TzAbstractItemModelPrivate::columnsInserted(const TzModelIndex &parent, int first, int last)
// {
//     std::vector<TzPersistentTzModelIndexData *> persistent_moved = persistent.moved.back();
//     persistent.moved.pop_back();

//     const int count = (last - first) + 1;
//     auto *q = q_func();

//     for (auto *data : persistent_moved) {
//         TzModelIndex old = data->index;

//         auto range = persistent.indexes.equal_range(old);
//         for (auto it = range.first; it != range.second; ++it) {
//             if (it->second == data) {
//                 persistent.indexes.erase(it);
//                 break;
//             }
//         }

//         data->index = q->index(old.row(), old.column() + count, parent);
//         if (data->index.isValid()) {
//             persistent.insertMultiAtEnd(data->index, data);
//         }
//     }
// }

// void TzAbstractItemModelPrivate::columnsAboutToBeRemoved(const TzModelIndex &parent, int first,
//                                                          int last)
// {
//     std::vector<TzPersistentTzModelIndexData *> persistent_moved;
//     std::vector<TzPersistentTzModelIndexData *> persistent_invalidated;

//     for (const auto &pair : persistent.indexes) {
//         TzPersistentTzModelIndexData *data = pair.second;
//         const TzModelIndex &index = data->index;

//         if (index.isValid() && index.parent() == parent) {
//             if (index.column() >= first && index.column() <= last) {
//                 persistent_invalidated.push_back(data);
//             } else if (index.column() > last) {
//                 persistent_moved.push_back(data);
//             }
//         }
//     }

//     persistent.moved.push_back(persistent_moved);
//     persistent.invalidated.push_back(persistent_invalidated);
// }

// void TzAbstractItemModelPrivate::columnsRemoved(const TzModelIndex &parent, int first, int last)
// {
//     std::vector<TzPersistentTzModelIndexData *> persistent_moved = persistent.moved.back();
//     persistent.moved.pop_back();

//     std::vector<TzPersistentTzModelIndexData *> persistent_invalidated
//         = persistent.invalidated.back();
//     persistent.invalidated.pop_back();

//     const int count = (last - first) + 1;
//     auto *q = q_func();

//     for (auto *data : persistent_invalidated) {
//         auto range = persistent.indexes.equal_range(data->index);
//         for (auto it = range.first; it != range.second; ++it) {
//             if (it->second == data) {
//                 persistent.indexes.erase(it);
//                 break;
//             }
//         }
//         data->index = TzModelIndex();
//     }

//     for (auto *data : persistent_moved) {
//         TzModelIndex old = data->index;

//         auto range = persistent.indexes.equal_range(old);
//         for (auto it = range.first; it != range.second; ++it) {
//             if (it->second == data) {
//                 persistent.indexes.erase(it);
//                 break;
//             }
//         }

//         data->index = q->index(old.row(), old.column() - count, parent);
//         if (data->index.isValid()) {
//             persistent.insertMultiAtEnd(data->index, data);
//         }
//     }
// }

// #define TZ_UNUSED(x) (void) x

// // Move validation (Qt-accurate)
// bool TzAbstractItemModelPrivate::allowMove(const TzModelIndex &srcParent, int start, int end,
//                                            const TzModelIndex &destinationParent,
//                                            int destinationStart,
//                                            TzAbstractItemModel::Orientation orientation)
// {
//     // Qt's validation logic
//     if (start < 0 || start > end)
//         return false;

//     auto *q = q_func();
//     int count = (orientation == TzAbstractItemModel::Orientation::Vertical)
//                     ? q->rowCount(srcParent)
//                     : q->columnCount(srcParent);

//     if (end >= count)
//         return false;

//     if (destinationStart < 0)
//         return false;

//     int destCount = (orientation == TzAbstractItemModel::Orientation::Vertical)
//                         ? q->rowCount(destinationParent)
//                         : q->columnCount(destinationParent);

//     if (destinationStart > destCount)
//         return false;

//     // Can't move to within itself
//     if (srcParent == destinationParent) {
//         if (destinationStart >= start && destinationStart <= end + 1)
//             return false;
//     }

//     return true;
// }

// void TzAbstractItemModelPrivate::itemsAboutToBeMoved(const TzModelIndex &sourceParent,
//                                                      int sourceFirst, int sourceLast,
//                                                      const TzModelIndex &destinationParent,
//                                                      int destinationChild,
//                                                      TzAbstractItemModel::Orientation orientation)
// {
//     // This is complex in Qt - simplified version for now
//     // Would need full implementation for move operations
//     TZ_UNUSED(sourceParent);
//     TZ_UNUSED(sourceFirst);
//     TZ_UNUSED(sourceLast);
//     TZ_UNUSED(destinationParent);
//     TZ_UNUSED(destinationChild);
//     TZ_UNUSED(orientation);
// }

// void TzAbstractItemModelPrivate::itemsMoved(const TzModelIndex &sourceParent, int sourceFirst,
//                                             int sourceLast, const TzModelIndex &destinationParent,
//                                             int destinationChild,
//                                             TzAbstractItemModel::Orientation orientation)
// {
//     // Complex persistent index updates for moves
//     TZ_UNUSED(sourceParent);
//     TZ_UNUSED(sourceFirst);
//     TZ_UNUSED(sourceLast);
//     TZ_UNUSED(destinationParent);
//     TZ_UNUSED(destinationChild);
//     TZ_UNUSED(orientation);
// }

// void TzAbstractItemModelPrivate::movePersistentIndexes(
//     const std::vector<TzPersistentTzModelIndexData *> &indexes, int change,
//     const TzModelIndex &parent, TzAbstractItemModel::Orientation orientation)
// {
//     TZ_UNUSED(indexes);
//     TZ_UNUSED(change);
//     TZ_UNUSED(parent);
//     TZ_UNUSED(orientation);
// }

// TzAbstractItemModel::TzAbstractItemModel()
//     : d_ptr(new TzAbstractItemModelPrivate)
// {
//     d_ptr->q_ptr = this;
// }

// TzAbstractItemModel::~TzAbstractItemModel() {}

// bool TzAbstractItemModel::setData(const TzModelIndex &index, const std::any &value, int role)
// {
//     (void) index;
//     (void) value;
//     (void) role;
//     return false;
// }

// std::any TzAbstractItemModel::headerData(int section, Orientation orientation, int role) const
// {
//     (void) role;
//     if (orientation == Orientation::Horizontal)
//         return std::string("Column ") + std::to_string(section);
//     return std::string("Row ") + std::to_string(section);
// }

// bool TzAbstractItemModel::setHeaderData(int section, Orientation orientation, const std::any &value,
//                                         int role)
// {
//     (void) section;
//     (void) orientation;
//     (void) value;
//     (void) role;
//     return false;
// }

// TzItemFlag TzAbstractItemModel::flags(const TzModelIndex &index) const
// {
//     if (!index.isValid())
//         return TzItemFlag::NoItemFlags;
//     return TzItemFlag::IsSelectable | TzItemFlag::IsEnabled;
// }

// bool TzAbstractItemModel::insertRows(int row, int count, const TzModelIndex &parent)
// {
//     (void) row;
//     (void) count;
//     (void) parent;
//     return false;
// }

// bool TzAbstractItemModel::insertColumns(int column, int count, const TzModelIndex &parent)
// {
//     (void) column;
//     (void) count;
//     (void) parent;
//     return false;
// }

// bool TzAbstractItemModel::removeRows(int row, int count, const TzModelIndex &parent)
// {
//     (void) row;
//     (void) count;
//     (void) parent;
//     return false;
// }

// bool TzAbstractItemModel::removeColumns(int column, int count, const TzModelIndex &parent)
// {
//     (void) column;
//     (void) count;
//     (void) parent;
//     return false;
// }

// bool TzAbstractItemModel::moveRows(const TzModelIndex &sourceParent, int sourceRow, int count,
//                                    const TzModelIndex &destinationParent, int destinationChild)
// {
//     (void) sourceParent;
//     (void) sourceRow;
//     (void) count;
//     (void) destinationParent;
//     (void) destinationChild;
//     return false;
// }

// bool TzAbstractItemModel::moveColumns(const TzModelIndex &sourceParent, int sourceColumn, int count,
//                                       const TzModelIndex &destinationParent, int destinationChild)
// {
//     (void) sourceParent;
//     (void) sourceColumn;
//     (void) count;
//     (void) destinationParent;
//     (void) destinationChild;
//     return false;
// }

// bool TzAbstractItemModel::hasIndex(int row, int column, const TzModelIndex &parent) const
// {
//     if (row < 0 || column < 0)
//         return false;
//     return row < rowCount(parent) && column < columnCount(parent);
// }

// TzModelIndex TzAbstractItemModel::sibling(int row, int column, const TzModelIndex &index) const
// {
//     return this->index(row, column, parent(index));
// }

// bool TzAbstractItemModel::hasChildren(const TzModelIndex &parent) const
// {
//     return rowCount(parent) > 0 && columnCount(parent) > 0;
// }

// void TzAbstractItemModel::sort(int column, bool ascending)
// {
//     (void) column;
//     (void) ascending;
// }

// std::vector<std::string> TzAbstractItemModel::mimeTypes() const
// {
//     return {};
// }

// std::any TzAbstractItemModel::mimeData(const std::vector<TzModelIndex> &indexes) const
// {
//     (void) indexes;
//     return std::any();
// }

// bool TzAbstractItemModel::canDropMimeData(const std::any &data, int action, int row, int column,
//                                           const TzModelIndex &parent) const
// {
//     (void) data;
//     (void) action;
//     (void) row;
//     (void) column;
//     (void) parent;
//     return false;
// }

// bool TzAbstractItemModel::dropMimeData(const std::any &data, int action, int row, int column,
//                                        const TzModelIndex &parent)
// {
//     (void) data;
//     (void) action;
//     (void) row;
//     (void) column;
//     (void) parent;
//     return false;
// }

// TzAbstractItemModel::TzAbstractItemModel(TzAbstractItemModelPrivate &dd)
//     : d_ptr(&dd)
// {
//     d_ptr->q_ptr = this;
// }

// TzModelIndex TzAbstractItemModel::createIndex(int row, int column, void *ptr) const
// {
//     return TzModelIndex(row, column, ptr, this);
// }

// TzModelIndex TzAbstractItemModel::createIndex(int row, int column, TzModelIndexId id) const
// {
//     return TzModelIndex(row, column, reinterpret_cast<void *>(id), this);
// }

// void TzAbstractItemModel::emitDataChanged(const TzModelIndex &topLeft,
//                                           const TzModelIndex &bottomRight,
//                                           const std::vector<int> &roles)
// {
//     m_events.emit("dataChanged", topLeft, bottomRight, roles);
// }

// void TzAbstractItemModel::emitHeaderDataChanged(Orientation orientation, int first, int last)
// {
//     m_events.emit("headerDataChanged", static_cast<int>(orientation), first, last);
// }

// void TzAbstractItemModel::changePersistentIndex(const TzModelIndex &from, const TzModelIndex &to)
// {
//     auto *d = d_func();

//     auto range = d->persistent.indexes.equal_range(from);
//     std::vector<TzPersistentTzModelIndexData *> datas;
//     for (auto it = range.first; it != range.second; ++it) {
//         datas.push_back(it->second);
//     }
//     d->persistent.indexes.erase(from);

//     for (auto *data : datas) {
//         data->index = to;
//         if (to.isValid()) {
//             d->persistent.insertMultiAtEnd(to, data);
//         }
//     }
// }

// void TzAbstractItemModel::changePersistentIndexList(const std::vector<TzModelIndex> &from,
//                                                     const std::vector<TzModelIndex> &to)
// {
//     if (from.size() != to.size()) {
//         throw std::invalid_argument("from and to lists must have same size");
//     }

//     for (size_t i = 0; i < from.size(); ++i) {
//         changePersistentIndex(from[i], to[i]);
//     }
// }

// std::vector<TzModelIndex> TzAbstractItemModel::persistentIndexList() const
// {
//     auto *d = d_func();
//     std::vector<TzModelIndex> result;
//     result.reserve(d->persistent.indexes.size());

//     for (const auto &pair : d->persistent.indexes) {
//         if (pair.second->index.isValid()) {
//             result.push_back(pair.second->index);
//         }
//     }

//     return result;
// }

// Copyright (C) 2022 The Qt Company Ltd.
// Copyright (C) 2020 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author Giuseppe D'Angelo <giuseppe.dangelo@kdab.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "TzAbstractItemModel.h"
#include <private/TzAbstractItemModel_p.h>
#include <qcollator.h>
#include <qdatastream.h>
#include <qstringlist.h>
#include <qsize.h>
#include <qmimedata.h>
#include <qdebug.h>
#include <std::list.h>
#if QT_CONFIG(regularexpression)
#  include <qregularexpression.h>
#endif
#include <qstack.h>
#include <std::unordered_map.h>
#include <qbitarray.h>
#include <qdatetime.h>
#include <qloggingcategory.h>

#include <functional>

#include <limits.h>

QT_BEGIN_NAMESPACE

Q_STATIC_LOGGING_CATEGORY(lcCheckIndex, "qt.core.TzAbstractItemModel.checkindex")
Q_STATIC_LOGGING_CATEGORY(lcReset, "qt.core.TzAbstractItemModel.reset")

QT_IMPL_METATYPE_EXTERN(TzModelIndexList)

TzPersistentModelIndexData *TzPersistentModelIndexData::create(const TzModelIndex &index)
{
    TZ_ASSERT(index.isValid()); // we will _never_ insert an invalid index in the list
    TzPersistentModelIndexData *d = nullptr;
    TzAbstractItemModel *model = const_cast<TzAbstractItemModel *>(index.model());
    QMultiHash<QtPrivate::TzModelIndexWrapper, TzPersistentModelIndexData *> &indexes = model->d_func()->persistent.indexes;
    const auto it = indexes.constFind(index);
    if (it != indexes.cend()) {
        d = (*it);
    } else {
        d = new TzPersistentModelIndexData(index);
        indexes.insert(index, d);
    }
    TZ_ASSERT(d);
    return d;
}

void TzPersistentModelIndexData::destroy(TzPersistentModelIndexData *data)
{
    TZ_ASSERT(data);
    TZ_ASSERT(data->ref.loadRelaxed() == 0);
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
    if (d) d->ref.ref();
}

TzPersistentModelIndex::TzPersistentModelIndex(const TzModelIndex &index)
    : d(nullptr)
{
    if (index.isValid()) {
        d = TzPersistentModelIndexData::create(index);
        d->ref.ref();
    }
}

TzPersistentModelIndex::~TzPersistentModelIndex()
{
    if (d && !d->ref.deref()) {
        TzPersistentModelIndexData::destroy(d);
        d = nullptr;
    }
}

bool comparesEqual(const TzPersistentModelIndex &lhs, const TzPersistentModelIndex &rhs) noexcept
{
    if (lhs.d && rhs.d)
        return lhs.d->index == rhs.d->index;
    return lhs.d == rhs.d;
}

Qt::strong_ordering compareThreeWay(const TzPersistentModelIndex &lhs,
                                    const TzPersistentModelIndex &rhs) noexcept
{
    if (lhs.d && rhs.d)
        return compareThreeWay(lhs.d->index, rhs.d->index);

    using Qt::totally_ordered_wrapper;
    return compareThreeWay(totally_ordered_wrapper{lhs.d}, totally_ordered_wrapper{rhs.d});
}

Qt::strong_ordering compareThreeWay(const TzPersistentModelIndex &lhs,
                                    const TzModelIndex &rhs) noexcept
{
    return compareThreeWay(lhs.d ? lhs.d->index : TzModelIndex{}, rhs);
}

TzPersistentModelIndex &TzPersistentModelIndex::operator=(const TzPersistentModelIndex &other)
{
    if (d == other.d)
        return *this;
    if (d && !d->ref.deref())
        TzPersistentModelIndexData::destroy(d);
    d = other.d;
    if (d) d->ref.ref();
    return *this;
}

TzPersistentModelIndex &TzPersistentModelIndex::operator=(const TzModelIndex &other)
{
    if (d && !d->ref.deref())
        TzPersistentModelIndexData::destroy(d);
    if (other.isValid()) {
        d = TzPersistentModelIndexData::create(other);
        if (d) d->ref.ref();
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

bool comparesEqual(const TzPersistentModelIndex &lhs, const TzModelIndex &rhs) noexcept
{
    if (lhs.d)
        return lhs.d->index == rhs;
    return !rhs.isValid();
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

quintptr TzPersistentModelIndex::internalId() const
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

Qt::ItemFlags TzPersistentModelIndex::flags() const
{
    if (d)
        return d->index.flags();
    return {};
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
    for (TzPersistentModelIndexData *data : std::as_const(persistent.indexes))
        data->index = TzModelIndex();
    persistent.indexes.clear();
}

void TzAbstractItemModelPrivate::invalidatePersistentIndex(const TzModelIndex &index) {
    const auto it = persistent.indexes.constFind(index);
    if (it != persistent.indexes.cend()) {
        TzPersistentModelIndexData *data = *it;
        persistent.indexes.erase(it);
        data->index = TzModelIndex();
    }
}

using DefaultRoleNames = std::unordered_map<int, std::string>;
TZ_GLOBAL_STATIC(DefaultRoleNames, qDefaultRoleNames,
    {
        { Qt::DisplayRole, "display" },
        { Qt::DecorationRole, "decoration" },
        { Qt::EditRole, "edit" },
        { Qt::ToolTipRole, "toolTip" },
        { Qt::StatusTipRole, "statusTip" },
        { Qt::WhatsThisRole, "whatsThis" },
    })

const std::unordered_map<int, std::string> &TzAbstractItemModelPrivate::defaultRoleNames()
{
    return *qDefaultRoleNames();
}

Qt::weak_ordering TzAbstractItemModel::compareData(const std::any &left, const std::any &right,
                                                     const QCollator *collator)
{
    // invalid is greater than everything, except another invalid variant
    if (!left.isValid()) {
        if (!right.isValid())
            return Qt::weak_ordering::equivalent;
        return Qt::weak_ordering::greater;
    }
    if (!right.isValid())
        return Qt::weak_ordering::less;
    switch (left.userType()) {
    case QMetaType::Int:
    case QMetaType::UInt:
    case QMetaType::LongLong:
    case QMetaType::ULongLong:
    case QMetaType::Float:
    case QMetaType::Double:
    case QMetaType::QChar:
    case QMetaType::QDate:
    case QMetaType::QTime:
    case QMetaType::QDateTime: {
        QPartialOrdering partialOrder = std::any::compare(left, right);
        if (partialOrder == Qt::partial_ordering::unordered) {
            if (right.canConvert(left.metaType())) {
                std::any rightAsLeft = right;
                rightAsLeft.convert(left.metaType());
                partialOrder = std::any::compare(left, rightAsLeft);
            }
            if (partialOrder == Qt::partial_ordering::unordered)
                return Qt::weak_ordering::greater;
        }
        if (partialOrder == Qt::partial_ordering::equivalent)
            return Qt::weak_ordering::equivalent;
        return partialOrder < 0 ? Qt::weak_ordering::less
                                : Qt::weak_ordering::greater;
    }
    default:
    case QMetaType::QString: {
        const Qt::CaseSensitivity cs = collator ? collator->caseSensitivity()
                                     : Qt::CaseSensitive;
        const int res = collator
                      ? collator->compare(left.toString(), right.toString())
                      : left.toString().compare(right.toString(), cs);
        return Qt::compareThreeWay(res, 0);
    }
    }
}

static uint typeOfVariant(const std::any &value)
{
    //return 0 for integer, 1 for floating point and 2 for other
    switch (value.userType()) {
        case QMetaType::Bool:
        case QMetaType::Int:
        case QMetaType::UInt:
        case QMetaType::LongLong:
        case QMetaType::ULongLong:
        case QMetaType::QChar:
        case QMetaType::Short:
        case QMetaType::UShort:
        case QMetaType::UChar:
        case QMetaType::ULong:
        case QMetaType::Long:
            return 0;
        case QMetaType::Double:
        case QMetaType::Float:
            return 1;
        default:
            return 2;
    }
}

bool TzAbstractItemModelPrivate::variantLessThan(const std::any &v1, const std::any &v2)
{
    switch(qMax(typeOfVariant(v1), typeOfVariant(v2)))
    {
    case 0: //integer type
        return v1.toLongLong() < v2.toLongLong();
    case 1: //floating point
        return v1.toReal() < v2.toReal();
    default:
        return v1.toString().localeAwareCompare(v2.toString()) < 0;
    }
}

void TzAbstractItemModelPrivate::removePersistentIndexData(TzPersistentModelIndexData *data)
{
    if (data->index.isValid()) {
        int removed = persistent.indexes.remove(data->index);
        TZ_ASSERT_X(removed == 1, "TzPersistentModelIndex::~TzPersistentModelIndex",
                   "persistent model indexes corrupted"); //maybe the index was somewhat invalid?
        // This assert may happen if the model use changePersistentIndex in a way that could result on two
        // TzPersistentModelIndex pointing to the same index.
        TZ_UNUSED(removed);
    }
    // make sure our optimization still works
    for (int i = persistent.moved.size() - 1; i >= 0; --i) {
        int idx = persistent.moved.at(i).indexOf(data);
        if (idx >= 0)
            persistent.moved[i].remove(idx);
    }
    // update the references to invalidated persistent indexes
    for (int i = persistent.invalidated.size() - 1; i >= 0; --i) {
        int idx = persistent.invalidated.at(i).indexOf(data);
        if (idx >= 0)
            persistent.invalidated[i].remove(idx);
    }
}

void TzAbstractItemModelPrivate::rowsAboutToBeInserted(const TzModelIndex &parent, int first, int last)
{
    TZ_Q(TzAbstractItemModel);
    TZ_UNUSED(last);
    std::list<TzPersistentModelIndexData *> persistent_moved;
    if (first < q->rowCount(parent)) {
        for (auto *data : std::as_const(persistent.indexes)) {
            const TzModelIndex &index = data->index;
            if (index.row() >= first && index.isValid() && index.parent() == parent) {
                persistent_moved.append(data);
            }
        }
    }
    persistent.moved.push(persistent_moved);
}

void TzAbstractItemModelPrivate::rowsInserted(const TzModelIndex &parent,
                                             int first, int last)
{
    const std::list<TzPersistentModelIndexData *> persistent_moved = persistent.moved.pop();
    const int count = (last - first) + 1; // it is important to only use the delta, because the change could be nested
    for (auto *data : persistent_moved) {
        TzModelIndex old = data->index;
        persistent.indexes.erase(persistent.indexes.constFind(old));
        data->index = q_func()->index(old.row() + count, old.column(), parent);
        if (data->index.isValid()) {
            persistent.insertMultiAtEnd(data->index, data);
        } else {
            qWarning() << "TzAbstractItemModel::endInsertRows:  Invalid index (" << old.row() + count << ',' << old.column() << ") in model" << q_func();
        }
    }
}

void TzAbstractItemModelPrivate::itemsAboutToBeMoved(const TzModelIndex &srcParent, int srcFirst, int srcLast, const TzModelIndex &destinationParent, int destinationChild, Qt::Orientation orientation)
{
    std::list<TzPersistentModelIndexData *> persistent_moved_explicitly;
    std::list<TzPersistentModelIndexData *> persistent_moved_in_source;
    std::list<TzPersistentModelIndexData *> persistent_moved_in_destination;

    const bool sameParent = (srcParent == destinationParent);
    const bool movingUp = (srcFirst > destinationChild);

    for (auto *data : std::as_const(persistent.indexes)) {
        const TzModelIndex &index = data->index;
        const TzModelIndex &parent = index.parent();
        const bool isSourceIndex = (parent == srcParent);
        const bool isDestinationIndex = (parent == destinationParent);

        int childPosition;
        if (orientation == Qt::Vertical)
            childPosition = index.row();
        else
            childPosition = index.column();

        if (!index.isValid() || !(isSourceIndex || isDestinationIndex ) )
            continue;

        if (!sameParent && isDestinationIndex) {
            if (childPosition >= destinationChild)
                persistent_moved_in_destination.append(data);
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
            persistent_moved_explicitly.append(data);
        } else {
            persistent_moved_in_source.append(data);
        }
    }
    persistent.moved.push(persistent_moved_explicitly);
    persistent.moved.push(persistent_moved_in_source);
    persistent.moved.push(persistent_moved_in_destination);
}

void TzAbstractItemModelPrivate::movePersistentIndexes(const std::list<TzPersistentModelIndexData *> &indexes, int change,
                                                      const TzModelIndex &parent, Qt::Orientation orientation)
{
    for (auto *data : indexes) {
        int row = data->index.row();
        int column = data->index.column();

        if (Qt::Vertical == orientation)
            row += change;
        else
            column += change;

        persistent.indexes.erase(persistent.indexes.constFind(data->index));
        data->index = q_func()->index(row, column, parent);
        if (data->index.isValid()) {
            persistent.insertMultiAtEnd(data->index, data);
        } else {
            qWarning() << "TzAbstractItemModel::endMoveRows:  Invalid index (" << row << "," << column << ") in model" << q_func();
        }
    }
}

void TzAbstractItemModelPrivate::itemsMoved(const TzModelIndex &sourceParent, int sourceFirst, int sourceLast, const TzModelIndex &destinationParent, int destinationChild, Qt::Orientation orientation)
{
    const std::list<TzPersistentModelIndexData *> moved_in_destination = persistent.moved.pop();
    const std::list<TzPersistentModelIndexData *> moved_in_source = persistent.moved.pop();
    const std::list<TzPersistentModelIndexData *> moved_explicitly = persistent.moved.pop();

    const bool sameParent = (sourceParent == destinationParent);
    const bool movingUp = (sourceFirst > destinationChild);

    const int explicit_change = (!sameParent || movingUp) ? destinationChild - sourceFirst : destinationChild - sourceLast - 1 ;
    const int source_change = (!sameParent || !movingUp) ? -1*(sourceLast - sourceFirst + 1) : sourceLast - sourceFirst + 1 ;
    const int destination_change = sourceLast - sourceFirst + 1;

    movePersistentIndexes(moved_explicitly, explicit_change, destinationParent, orientation);
    movePersistentIndexes(moved_in_source, source_change, sourceParent, orientation);
    movePersistentIndexes(moved_in_destination, destination_change, destinationParent, orientation);
}

void TzAbstractItemModelPrivate::rowsAboutToBeRemoved(const TzModelIndex &parent,
                                                     int first, int last)
{
    std::list<TzPersistentModelIndexData *> persistent_moved;
    std::list<TzPersistentModelIndexData *> persistent_invalidated;
    // find the persistent indexes that are affected by the change, either by being in the removed subtree
    // or by being on the same level and below the removed rows
    for (auto *data : std::as_const(persistent.indexes)) {
        bool level_changed = false;
        TzModelIndex current = data->index;
        while (current.isValid()) {
            TzModelIndex current_parent = current.parent();
            if (current_parent == parent) { // on the same level as the change
                if (!level_changed && current.row() > last) // below the removed rows
                    persistent_moved.append(data);
                else if (current.row() <= last && current.row() >= first) // in the removed subtree
                    persistent_invalidated.append(data);
                break;
            }
            current = current_parent;
            level_changed = true;
        }
    }

    persistent.moved.push(persistent_moved);
    persistent.invalidated.push(persistent_invalidated);
}

void TzAbstractItemModelPrivate::rowsRemoved(const TzModelIndex &parent, int first, int last)
{
    const std::list<TzPersistentModelIndexData *> persistent_moved = persistent.moved.pop();
    const int count = (last - first) + 1; // it is important to only use the delta, because the change could be nested
    for (auto *data : persistent_moved) {
        TzModelIndex old = data->index;
        persistent.indexes.erase(persistent.indexes.constFind(old));
        data->index = q_func()->index(old.row() - count, old.column(), parent);
        if (data->index.isValid()) {
            persistent.insertMultiAtEnd(data->index, data);
        } else {
            qWarning() << "TzAbstractItemModel::endRemoveRows:  Invalid index (" << old.row() - count << ',' << old.column() << ") in model" << q_func();
        }
    }
    const std::list<TzPersistentModelIndexData *> persistent_invalidated = persistent.invalidated.pop();
    for (auto *data : persistent_invalidated) {
        auto pit = persistent.indexes.constFind(data->index);
        if (pit != persistent.indexes.cend())
            persistent.indexes.erase(pit);
        data->index = TzModelIndex();
    }
}

void TzAbstractItemModelPrivate::columnsAboutToBeInserted(const TzModelIndex &parent, int first, int last)
{
    TZ_Q(TzAbstractItemModel);
    TZ_UNUSED(last);
    std::list<TzPersistentModelIndexData *> persistent_moved;
    if (first < q->columnCount(parent)) {
        for (auto *data : std::as_const(persistent.indexes)) {
            const TzModelIndex &index = data->index;
            if (index.column() >= first && index.isValid() && index.parent() == parent)
                persistent_moved.append(data);
        }
    }
    persistent.moved.push(persistent_moved);
}

void TzAbstractItemModelPrivate::columnsInserted(const TzModelIndex &parent, int first, int last)
{
    const std::list<TzPersistentModelIndexData *> persistent_moved = persistent.moved.pop();
    const int count = (last - first) + 1; // it is important to only use the delta, because the change could be nested
    for (auto *data : persistent_moved) {
        TzModelIndex old = data->index;
        persistent.indexes.erase(persistent.indexes.constFind(old));
        data->index = q_func()->index(old.row(), old.column() + count, parent);
        if (data->index.isValid()) {
            persistent.insertMultiAtEnd(data->index, data);
        } else {
            qWarning() << "TzAbstractItemModel::endInsertColumns:  Invalid index (" << old.row() << ',' << old.column() + count << ") in model" << q_func();
        }
    }
}

void TzAbstractItemModelPrivate::columnsAboutToBeRemoved(const TzModelIndex &parent, int first, int last)
{
    std::list<TzPersistentModelIndexData *> persistent_moved;
    std::list<TzPersistentModelIndexData *> persistent_invalidated;
    // find the persistent indexes that are affected by the change, either by being in the removed subtree
    // or by being on the same level and to the right of the removed columns
    for (auto *data : std::as_const(persistent.indexes)) {
        bool level_changed = false;
        TzModelIndex current = data->index;
        while (current.isValid()) {
            TzModelIndex current_parent = current.parent();
            if (current_parent == parent) { // on the same level as the change
                if (!level_changed && current.column() > last) // right of the removed columns
                    persistent_moved.append(data);
                else if (current.column() <= last && current.column() >= first) // in the removed subtree
                    persistent_invalidated.append(data);
                break;
            }
            current = current_parent;
            level_changed = true;
        }
    }

    persistent.moved.push(persistent_moved);
    persistent.invalidated.push(persistent_invalidated);
}

void TzAbstractItemModelPrivate::columnsRemoved(const TzModelIndex &parent, int first, int last)
{
    const std::list<TzPersistentModelIndexData *> persistent_moved = persistent.moved.pop();
    const int count = (last - first) + 1; // it is important to only use the delta, because the change could be nested
    for (auto *data : persistent_moved) {
        TzModelIndex old = data->index;
        persistent.indexes.erase(persistent.indexes.constFind(old));
        data->index = q_func()->index(old.row(), old.column() - count, parent);
        if (data->index.isValid()) {
            persistent.insertMultiAtEnd(data->index, data);
        } else {
            qWarning() << "TzAbstractItemModel::endRemoveColumns:  Invalid index (" << old.row() << ',' << old.column() - count << ") in model" << q_func();
        }
    }
    const std::list<TzPersistentModelIndexData *> persistent_invalidated = persistent.invalidated.pop();
    for (auto *data : persistent_invalidated) {
        auto index = persistent.indexes.constFind(data->index);
        if (index != persistent.indexes.constEnd())
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
}

TzAbstractItemModel::TzAbstractItemModel(TzAbstractItemModelPrivate &dd)
    : d_ptr(dd)
{
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

std::unordered_map<int, std::any> TzAbstractItemModel::itemData(const TzModelIndex &index) const
{
    std::unordered_map<int, std::any> roles;
    for (int i = 0; i < Qt::UserRole; ++i) {
        std::any variantData = data(index, i);
        if (variantData.isValid())
            roles.insert(i, variantData);
    }
    return roles;
}

bool TzAbstractItemModel::setData(const TzModelIndex &index, const std::any &value, int role)
{
    TZ_UNUSED(index);
    TZ_UNUSED(value);
    TZ_UNUSED(role);
    return false;
}

bool TzAbstractItemModel::clearItemData(const TzModelIndex &index)
{
    TZ_UNUSED(index);
    return false;
}

bool TzAbstractItemModel::setItemData(const TzModelIndex &index, const std::unordered_map<int, std::any> &roles)
{
    if (!index.isValid() || roles.isEmpty())
        return false;

    // ### TODO: Consider change the semantics of this function,
    // or deprecating/removing it altogether.
    //
    // For instance, it should try setting *all* the data
    // in \a roles, and not bail out at the first setData that returns
    // false. It should also have a transactional approach.
    for (auto it = roles.begin(), e = roles.end(); it != e; ++it) {
        if (!setData(index, it.value(), it.key()))
            return false;
    }
    return true;
}

QStringList TzAbstractItemModel::mimeTypes() const
{
    QStringList types;
    types << QStringLiteral("application/x-TzAbstractItemModeldatalist");
    return types;
}

QMimeData *TzAbstractItemModel::mimeData(const TzModelIndexList &indexes) const
{
    if (indexes.size() <= 0)
        return nullptr;
    QStringList types = mimeTypes();
    if (types.isEmpty())
        return nullptr;
    QMimeData *data = new QMimeData();
    QString format = types.at(0);
    std::string encoded;
    QDataStream stream(&encoded, QDataStream::WriteOnly);
    encodeData(indexes, stream);
    data->setData(format, encoded);
    return data;
}

bool TzAbstractItemModel::canDropMimeData(const QMimeData *data, Qt::DropAction action,
                                         int row, int column,
                                         const TzModelIndex &parent) const
{
    TZ_UNUSED(row);
    TZ_UNUSED(column);
    TZ_UNUSED(parent);

    if (!(action & supportedDropActions()))
        return false;

    const QStringList modelTypes = mimeTypes();
    for (int i = 0; i < modelTypes.size(); ++i) {
        if (data->hasFormat(modelTypes.at(i)))
            return true;
    }
    return false;
}

bool TzAbstractItemModel::dropMimeData(const QMimeData *data, Qt::DropAction action,
                                      int row, int column, const TzModelIndex &parent)
{
    // check if the action is supported
    if (!data || !(action == Qt::CopyAction || action == Qt::MoveAction))
        return false;
    // check if the format is supported
    const QStringList types = mimeTypes();
    if (types.isEmpty())
        return false;
    const QString format = types.at(0);
    if (!data->hasFormat(format))
        return false;
    const bool dropOnItem = row == -1 && column == -1 && parent.isValid();
    if (!dropOnItem || !parent.flags().testFlag(Qt::ItemNeverHasChildren)) {
        // drop in between items, or on an item that cannot have children
        // -> insert new item
        if (row > rowCount(parent))
            row = rowCount(parent);
        if (row == -1)
            row = rowCount(parent);
        if (column == -1)
            column = 0;
    }
    // decode and insert
    std::string encoded = data->data(format);
    QDataStream stream(&encoded, QDataStream::ReadOnly);
    return decodeData(row, column, parent, stream);
}

Qt::DropActions TzAbstractItemModel::supportedDropActions() const
{
    return Qt::CopyAction;
}

Qt::DropActions TzAbstractItemModel::supportedDragActions() const
{
    return supportedDropActions();
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

Qt::ItemFlags TzAbstractItemModel::flags(const TzModelIndex &index) const
{
    TZ_D(const TzAbstractItemModel);
    if (!d->indexValid(index))
        return { };

    return Qt::ItemIsSelectable|Qt::ItemIsEnabled;
}

void TzAbstractItemModel::sort(int column, Qt::SortOrder order)
{
    TZ_UNUSED(column);
    TZ_UNUSED(order);
    // do nothing
}

TzModelIndex TzAbstractItemModel::buddy(const TzModelIndex &index) const
{
    return index;
}

TzModelIndexList TzAbstractItemModel::match(const TzModelIndex &start, int role,
                                          const std::any &value, int hits,
                                          Qt::MatchFlags flags) const
{
    TzModelIndexList result;
    uint matchType = (flags & Qt::MatchTypeMask).toInt();
    Qt::CaseSensitivity cs = flags & Qt::MatchCaseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive;
    bool recurse = flags.testAnyFlag(Qt::MatchRecursive);
    bool wrap = flags.testAnyFlag(Qt::MatchWrap);
    bool allHits = (hits == -1);
    QString text; // only convert to a string if it is needed
#if QT_CONFIG(regularexpression)
    QRegularExpression rx; // only create it if needed
#endif
    const int column = start.column();
    TzModelIndex p = parent(start);
    int from = start.row();
    int to = rowCount(p);

    // iterates twice if wrapping
    for (int i = 0; (wrap && i < 2) || (!wrap && i < 1); ++i) {
        for (int r = from; (r < to) && (allHits || result.size() < hits); ++r) {
            TzModelIndex idx = index(r, column, p);
            if (!idx.isValid())
                 continue;
            std::any v = data(idx, role);
            // std::any based matching
            if (matchType == Qt::MatchExactly) {
                if (value == v)
                    result.append(idx);
            } else { // QString or regular expression based matching
#if QT_CONFIG(regularexpression)
                if (matchType == Qt::MatchRegularExpression) {
                    if (rx.pattern().isEmpty()) {
                        if (value.userType() == QMetaType::QRegularExpression) {
                            rx = value.toRegularExpression();
                        } else {
                            rx.setPattern(value.toString());
                            if (cs == Qt::CaseInsensitive)
                                rx.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
                        }
                    }
                } else if (matchType == Qt::MatchWildcard) {
                    if (rx.pattern().isEmpty()) {
                        const QString pattern = QRegularExpression::wildcardToRegularExpression(value.toString(), QRegularExpression::NonPathWildcardConversion);
                        rx.setPattern(pattern);
                    }
                    if (cs == Qt::CaseInsensitive)
                        rx.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
                } else
#endif
                {
                    if (text.isEmpty()) // lazy conversion
                        text = value.toString();
                }

                QString t = v.toString();
                switch (matchType) {
#if QT_CONFIG(regularexpression)
                case Qt::MatchRegularExpression:
                case Qt::MatchWildcard:
                    if (t.contains(rx))
                        result.append(idx);
                    break;
#endif
                case Qt::MatchStartsWith:
                    if (t.startsWith(text, cs))
                        result.append(idx);
                    break;
                case Qt::MatchEndsWith:
                    if (t.endsWith(text, cs))
                        result.append(idx);
                    break;
                case Qt::MatchFixedString:
                    if (t.compare(text, cs) == 0)
                        result.append(idx);
                    break;
                case Qt::MatchContains:
                default:
                    if (t.contains(text, cs))
                        result.append(idx);
                }
            }
            if (recurse) {
                const auto parent = column != 0 ? idx.sibling(idx.row(), 0) : idx;
                if (hasChildren(parent)) { // search the hierarchy
                    result += match(index(0, column, parent), role,
                                    (text.isEmpty() ? value : text),
                                    (allHits ? -1 : hits - result.size()), flags);
                }
            }
        }
        // prepare for the next iteration
        from = 0;
        to = start.row();
    }
    return result;
}

QSize TzAbstractItemModel::span(const TzModelIndex &) const
{
    return QSize(1, 1);
}

std::unordered_map<int, std::string> TzAbstractItemModel::roleNames() const
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

std::any TzAbstractItemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    TZ_UNUSED(orientation);
    if (role == Qt::DisplayRole)
        return section + 1;
    return std::any();
}

bool TzAbstractItemModel::setHeaderData(int section, Qt::Orientation orientation,
                                       const std::any &value, int role)
{
    TZ_UNUSED(section);
    TZ_UNUSED(orientation);
    TZ_UNUSED(value);
    TZ_UNUSED(role);
    return false;
}

void TzAbstractItemModel::encodeData(const TzModelIndexList &indexes, QDataStream &stream) const
{
    for (const auto &index : indexes)
        stream << index.row() << index.column() << itemData(index);
}

bool TzAbstractItemModel::decodeData(int row, int column, const TzModelIndex &parent,
                                    QDataStream &stream)
{
    int top = std::numeric_limits<int>::max();
    int left = std::numeric_limits<int>::max();
    int bottom = 0;
    int right = 0;
    std::list<int> rows, columns;
    std::list<std::unordered_map<int, std::any>> data;

    while (!stream.atEnd()) {
        int r, c;
        std::unordered_map<int, std::any> v;
        stream >> r >> c >> v;
        rows.append(r);
        columns.append(c);
        data.append(v);
        top = qMin(r, top);
        left = qMin(c, left);
        bottom = qMax(r, bottom);
        right = qMax(c, right);
    }

    // insert the dragged items into the table, use a bit array to avoid overwriting items,
    // since items from different tables can have the same row and column
    int dragRowCount = 0;
    int dragColumnCount = right - left + 1;

    // Compute the number of continuous rows upon insertion and modify the rows to match
    std::list<int> rowsToInsert(bottom + 1);
    for (int i = 0; i < rows.size(); ++i)
        rowsToInsert[rows.at(i)] = 1;
    for (int i = 0; i < rowsToInsert.size(); ++i) {
        if (rowsToInsert.at(i) == 1){
            rowsToInsert[i] = dragRowCount;
            ++dragRowCount;
        }
    }
    for (int i = 0; i < rows.size(); ++i)
        rows[i] = top + rowsToInsert.at(rows.at(i));

    QBitArray isWrittenTo(dragRowCount * dragColumnCount);

    // make space in the table for the dropped data
    int colCount = columnCount(parent);
    if (colCount == 0) {
        insertColumns(colCount, dragColumnCount - colCount, parent);
        colCount = columnCount(parent);
    }
    insertRows(row, dragRowCount, parent);

    row = qMax(0, row);
    column = qMax(0, column);

    std::list<TzPersistentModelIndex> newIndexes(data.size());
    // set the data in the table
    for (int j = 0; j < data.size(); ++j) {
        int relativeRow = rows.at(j) - top;
        int relativeColumn = columns.at(j) - left;
        int destinationRow = relativeRow + row;
        int destinationColumn = relativeColumn + column;
        int flat = (relativeRow * dragColumnCount) + relativeColumn;
        // if the item was already written to, or we just can't fit it in the table, create a new row
        if (destinationColumn >= colCount || isWrittenTo.testBit(flat)) {
            destinationColumn = qBound(column, destinationColumn, qMax(colCount - 1, column));
            destinationRow = row + dragRowCount;
            insertRows(row + dragRowCount, 1, parent);
            flat = (dragRowCount * dragColumnCount) + relativeColumn;
            isWrittenTo.resize(++dragRowCount * dragColumnCount);
        }
        if (!isWrittenTo.testBit(flat)) {
            newIndexes[j] = index(destinationRow, destinationColumn, parent);
            isWrittenTo.setBit(flat);
        }
    }

    for(int k = 0; k < newIndexes.size(); k++) {
        if (newIndexes.at(k).isValid())
            setItemData(newIndexes.at(k), data.at(k));
    }

    return true;
}

void TzAbstractItemModel::beginInsertRows(const TzModelIndex &parent, int first, int last)
{
    TZ_ASSERT(first >= 0);
    TZ_ASSERT(first <= rowCount(parent)); // == is allowed, to insert at the end
    TZ_ASSERT(last >= first);
    TZ_D(TzAbstractItemModel);
    d->changes.push(TzAbstractItemModelPrivate::Change(parent, first, last));
    rowsAboutToBeInserted(parent, first, last, QPrivateSignal());
    d->rowsAboutToBeInserted(parent, first, last);
}

void TzAbstractItemModel::endInsertRows()
{
    TZ_D(TzAbstractItemModel);
    TzAbstractItemModelPrivate::Change change = d->changes.pop();
    d->rowsInserted(change.parent, change.first, change.last);
    rowsInserted(change.parent, change.first, change.last, QPrivateSignal());
}

void TzAbstractItemModel::beginRemoveRows(const TzModelIndex &parent, int first, int last)
{
    TZ_ASSERT(first >= 0);
    TZ_ASSERT(last >= first);
    TZ_ASSERT(last < rowCount(parent));
    TZ_D(TzAbstractItemModel);
    d->changes.push(TzAbstractItemModelPrivate::Change(parent, first, last));
    rowsAboutToBeRemoved(parent, first, last, QPrivateSignal());
    d->rowsAboutToBeRemoved(parent, first, last);
}

void TzAbstractItemModel::endRemoveRows()
{
    TZ_D(TzAbstractItemModel);
    TzAbstractItemModelPrivate::Change change = d->changes.pop();
    d->rowsRemoved(change.parent, change.first, change.last);
    rowsRemoved(change.parent, change.first, change.last, QPrivateSignal());
}

bool TzAbstractItemModelPrivate::allowMove(const TzModelIndex &srcParent, int start, int end, const TzModelIndex &destinationParent, int destinationStart, Qt::Orientation orientation)
{
    // Don't move the range within itself.
    if (destinationParent == srcParent)
        return !(destinationStart >= start && destinationStart <= end + 1);

    TzModelIndex destinationAncestor = destinationParent;
    int pos = (Qt::Vertical == orientation) ? destinationAncestor.row() : destinationAncestor.column();
    forever {
        if (destinationAncestor == srcParent) {
            if (pos >= start && pos <= end)
                return false;
            break;
        }

        if (!destinationAncestor.isValid())
          break;

        pos = (Qt::Vertical == orientation) ? destinationAncestor.row() : destinationAncestor.column();
        destinationAncestor = destinationAncestor.parent();
    }

    return true;
}

void TzAbstractItemModelPrivate::executePendingOperations() const {}

bool TzAbstractItemModel::beginMoveRows(const TzModelIndex &sourceParent, int sourceFirst, int sourceLast, const TzModelIndex &destinationParent, int destinationChild)
{
    TZ_ASSERT(sourceFirst >= 0);
    TZ_ASSERT(sourceLast >= sourceFirst);
    TZ_ASSERT(destinationChild >= 0);
    TZ_D(TzAbstractItemModel);

    if (!d->allowMove(sourceParent, sourceFirst, sourceLast, destinationParent, destinationChild, Qt::Vertical))
        return false;

    TzAbstractItemModelPrivate::Change sourceChange(sourceParent, sourceFirst, sourceLast);
    sourceChange.needsAdjust = sourceParent.isValid() && sourceParent.row() >= destinationChild && sourceParent.parent() == destinationParent;
    d->changes.push(sourceChange);
    int destinationLast = destinationChild + (sourceLast - sourceFirst);
    TzAbstractItemModelPrivate::Change destinationChange(destinationParent, destinationChild, destinationLast);
    destinationChange.needsAdjust = destinationParent.isValid() && destinationParent.row() >= sourceLast && destinationParent.parent() == sourceParent;
    d->changes.push(destinationChange);

    rowsAboutToBeMoved(sourceParent, sourceFirst, sourceLast, destinationParent, destinationChild, QPrivateSignal());
    d->itemsAboutToBeMoved(sourceParent, sourceFirst, sourceLast, destinationParent, destinationChild, Qt::Vertical);
    return true;
}

void TzAbstractItemModel::endMoveRows()
{
    TZ_D(TzAbstractItemModel);

    TzAbstractItemModelPrivate::Change insertChange = d->changes.pop();
    TzAbstractItemModelPrivate::Change removeChange = d->changes.pop();

    TzModelIndex adjustedSource = removeChange.parent;
    TzModelIndex adjustedDestination = insertChange.parent;

    const int numMoved = removeChange.last - removeChange.first + 1;
    if (insertChange.needsAdjust)
      adjustedDestination = createIndex(adjustedDestination.row() - numMoved, adjustedDestination.column(), adjustedDestination.internalPointer());

    if (removeChange.needsAdjust)
      adjustedSource = createIndex(adjustedSource.row() + numMoved, adjustedSource.column(), adjustedSource.internalPointer());

    d->itemsMoved(adjustedSource, removeChange.first, removeChange.last, adjustedDestination, insertChange.first, Qt::Vertical);

    rowsMoved(adjustedSource, removeChange.first, removeChange.last, adjustedDestination, insertChange.first, QPrivateSignal());
}

void TzAbstractItemModel::beginInsertColumns(const TzModelIndex &parent, int first, int last)
{
    TZ_ASSERT(first >= 0);
    TZ_ASSERT(first <= columnCount(parent)); // == is allowed, to insert at the end
    TZ_ASSERT(last >= first);
    TZ_D(TzAbstractItemModel);
    d->changes.push(TzAbstractItemModelPrivate::Change(parent, first, last));
    columnsAboutToBeInserted(parent, first, last, QPrivateSignal());
    d->columnsAboutToBeInserted(parent, first, last);
}

void TzAbstractItemModel::endInsertColumns()
{
    TZ_D(TzAbstractItemModel);
    TzAbstractItemModelPrivate::Change change = d->changes.pop();
    d->columnsInserted(change.parent, change.first, change.last);
    columnsInserted(change.parent, change.first, change.last, QPrivateSignal());
}

void TzAbstractItemModel::beginRemoveColumns(const TzModelIndex &parent, int first, int last)
{
    TZ_ASSERT(first >= 0);
    TZ_ASSERT(last >= first);
    TZ_ASSERT(last < columnCount(parent));
    TZ_D(TzAbstractItemModel);
    d->changes.push(TzAbstractItemModelPrivate::Change(parent, first, last));
    columnsAboutToBeRemoved(parent, first, last, QPrivateSignal());
    d->columnsAboutToBeRemoved(parent, first, last);
}

void TzAbstractItemModel::endRemoveColumns()
{
    TZ_D(TzAbstractItemModel);
    TzAbstractItemModelPrivate::Change change = d->changes.pop();
    d->columnsRemoved(change.parent, change.first, change.last);
    columnsRemoved(change.parent, change.first, change.last, QPrivateSignal());
}

bool TzAbstractItemModel::beginMoveColumns(const TzModelIndex &sourceParent, int sourceFirst, int sourceLast, const TzModelIndex &destinationParent, int destinationChild)
{
    TZ_ASSERT(sourceFirst >= 0);
    TZ_ASSERT(sourceLast >= sourceFirst);
    TZ_ASSERT(destinationChild >= 0);
    TZ_D(TzAbstractItemModel);

    if (!d->allowMove(sourceParent, sourceFirst, sourceLast, destinationParent, destinationChild, Qt::Horizontal))
        return false;

    TzAbstractItemModelPrivate::Change sourceChange(sourceParent, sourceFirst, sourceLast);
    sourceChange.needsAdjust = sourceParent.isValid() && sourceParent.row() >= destinationChild && sourceParent.parent() == destinationParent;
    d->changes.push(sourceChange);
    int destinationLast = destinationChild + (sourceLast - sourceFirst);
    TzAbstractItemModelPrivate::Change destinationChange(destinationParent, destinationChild, destinationLast);
    destinationChange.needsAdjust = destinationParent.isValid() && destinationParent.row() >= sourceLast && destinationParent.parent() == sourceParent;
    d->changes.push(destinationChange);

    columnsAboutToBeMoved(sourceParent, sourceFirst, sourceLast, destinationParent, destinationChild, QPrivateSignal());
    d->itemsAboutToBeMoved(sourceParent, sourceFirst, sourceLast, destinationParent, destinationChild, Qt::Horizontal);
    return true;
}

void TzAbstractItemModel::endMoveColumns()
{
    TZ_D(TzAbstractItemModel);

    TzAbstractItemModelPrivate::Change insertChange = d->changes.pop();
    TzAbstractItemModelPrivate::Change removeChange = d->changes.pop();

    TzModelIndex adjustedSource = removeChange.parent;
    TzModelIndex adjustedDestination = insertChange.parent;

    const int numMoved = removeChange.last - removeChange.first + 1;
    if (insertChange.needsAdjust)
      adjustedDestination = createIndex(adjustedDestination.row(), adjustedDestination.column() - numMoved, adjustedDestination.internalPointer());

    if (removeChange.needsAdjust)
      adjustedSource = createIndex(adjustedSource.row(), adjustedSource.column() + numMoved, adjustedSource.internalPointer());

    d->itemsMoved(adjustedSource, removeChange.first, removeChange.last, adjustedDestination, insertChange.first, Qt::Horizontal);
    columnsMoved(adjustedSource, removeChange.first, removeChange.last, adjustedDestination, insertChange.first, QPrivateSignal());
}

void TzAbstractItemModel::beginResetModel()
{
    TZ_D(TzAbstractItemModel);
    if (d->resetting) {
        qWarning() << "beginResetModel called on" << this << "without calling endResetModel first";
        // Warn, but don't return early in case user code relies on the incorrect behavior.
    }

    qCDebug(lcReset) << "beginResetModel called; about to modelAboutToBeReset";
    d->resetting = true;
    modelAboutToBeReset(QPrivateSignal());
}

void TzAbstractItemModel::endResetModel()
{
    TZ_D(TzAbstractItemModel);
    if (!d->resetting) {
        qWarning() << "endResetModel called on" << this << "without calling beginResetModel first";
        // Warn, but don't return early in case user code relies on the incorrect behavior.
    }

    qCDebug(lcReset) << "endResetModel called; about to modelReset";
    d->invalidatePersistentIndexes();
    resetInternalData();
    d->resetting = false;
    modelReset(QPrivateSignal());
}

void TzAbstractItemModel::changePersistentIndex(const TzModelIndex &from, const TzModelIndex &to)
{
    TZ_D(TzAbstractItemModel);
    if (d->persistent.indexes.isEmpty())
        return;
    // find the data and reinsert it sorted
    const auto it = d->persistent.indexes.constFind(from);
    if (it != d->persistent.indexes.cend()) {
        TzPersistentModelIndexData *data = *it;
        d->persistent.indexes.erase(it);
        data->index = to;
        if (to.isValid())
            d->persistent.insertMultiAtEnd(to, data);
    }
}

void TzAbstractItemModel::changePersistentIndexList(const TzModelIndexList &from,
                                                   const TzModelIndexList &to)
{
    TZ_D(TzAbstractItemModel);
    if (d->persistent.indexes.isEmpty())
        return;
    std::list<TzPersistentModelIndexData *> toBeReinserted;
    toBeReinserted.reserve(to.size());
    for (int i = 0; i < from.size(); ++i) {
        if (from.at(i) == to.at(i))
            continue;
        const auto it = d->persistent.indexes.constFind(from.at(i));
        if (it != d->persistent.indexes.cend()) {
            TzPersistentModelIndexData *data = *it;
            d->persistent.indexes.erase(it);
            data->index = to.at(i);
            if (data->index.isValid())
                toBeReinserted << data;
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
    for (auto *data : std::as_const(d->persistent.indexes))
        result.append(data->index);
    return result;
}

bool TzAbstractItemModel::checkIndex(const TzModelIndex &index, CheckIndexOptions options) const
{
    if (!index.isValid()) {
        if (options & CheckIndexOption::IndexIsValid) {
            qCWarning(lcCheckIndex) << "Index" << index << "is not valid (expected valid)";
            return false;
        }
        return true;
    }

    if (index.model() != this) {
        qCWarning(lcCheckIndex) << "Index" << index
                                << "is for model" << index.model()
                                << "which is different from this model" << this;
        return false;
    }

    if (index.row() < 0) {
        qCWarning(lcCheckIndex) << "Index" << index
                                << "has negative row" << index.row();
        return false;
    }

    if (index.column() < 0) {
        qCWarning(lcCheckIndex) << "Index" << index
                                << "has negative column" << index.column();
        return false;
    }

    if (!(options & CheckIndexOption::DoNotUseParent)) {
        const TzModelIndex parentIndex = index.parent();
        if (options & CheckIndexOption::ParentIsInvalid) {
            if (parentIndex.isValid()) {
                qCWarning(lcCheckIndex) << "Index" << index
                                        << "has valid parent" << parentIndex
                                        << "(expected an invalid parent)";
                return false;
            }
        }

        const int rc = rowCount(parentIndex);
        if (index.row() >= rc) {
            qCWarning(lcCheckIndex) << "Index" << index
                                    << "has out of range row" << index.row()
                                    << "rowCount() is" << rc;
            return false;
        }

        const int cc = columnCount(parentIndex);
        if (index.column() >= cc) {
            qCWarning(lcCheckIndex) << "Index" << index
                                    << "has out of range column" << index.column()
                                    << "columnCount() is" << cc;
            return false;

        }
    }

    return true;
}

QAbstractListModel::QAbstractListModel()
{
}

QAbstractListModel::QAbstractListModel(TzAbstractItemModelPrivate &dd)
    : TzAbstractItemModel(dd)
{
}

QAbstractListModel::~QAbstractListModel()
{
}

TzModelIndex QAbstractListModel::index(int row, int column, const TzModelIndex &parent) const
{
    return hasIndex(row, column, parent) ? createIndex(row, column) : TzModelIndex();
}

TzModelIndex QAbstractListModel::parent(const TzModelIndex &index) const
{
    TZ_UNUSED(index);
    return TzModelIndex();
}

TzModelIndex QAbstractListModel::sibling(int row, int column, const TzModelIndex &) const
{
    return index(row, column);
}

Qt::ItemFlags QAbstractListModel::flags(const TzModelIndex &index) const
{
    Qt::ItemFlags f = TzAbstractItemModel::flags(index);
    if (index.isValid())
        f |= Qt::ItemNeverHasChildren;
    return f;
}

int QAbstractListModel::columnCount(const TzModelIndex &parent) const
{
    return parent.isValid() ? 0 : 1;
}

bool QAbstractListModel::hasChildren(const TzModelIndex &parent) const
{
    return parent.isValid() ? false : (rowCount() > 0);
}

bool TzAbstractItemModelPrivate::dropOnItem(const TzModelIndex &index, QDataStream &stream)
{
    TZ_Q(TzAbstractItemModel);

    int top = std::numeric_limits<int>::max();
    int left = std::numeric_limits<int>::max();
    std::list<int> rows, columns;
    std::list<std::unordered_map<int, std::any>> data;

    while (!stream.atEnd()) {
        int r, c;
        std::unordered_map<int, std::any> v;
        stream >> r >> c >> v;
        rows.append(r);
        columns.append(c);
        data.append(v);
        top = qMin(r, top);
        left = qMin(c, left);
    }

    for (int i = 0; i < data.size(); ++i) {
        int r = (rows.at(i) - top) + index.row();
        int c = (columns.at(i) - left) + index.column();
        if (q->hasIndex(r, c))
            q->setItemData(q->index(r, c), data.at(i));
    }

    return true;
}

bool QAbstractTableModel::dropMimeData(const QMimeData *data, Qt::DropAction action,
                                       int row, int column, const TzModelIndex &parent)
{
    TZ_D(TzAbstractItemModel);
    if (!data || !(action == Qt::CopyAction || action == Qt::MoveAction))
        return false;

    QStringList types = mimeTypes();
    if (types.isEmpty())
        return false;
    QString format = types.at(0);
    if (!data->hasFormat(format))
        return false;

    std::string encoded = data->data(format);
    QDataStream stream(&encoded, QDataStream::ReadOnly);

    // if the drop is on an item, replace the data in the items
    if (parent.isValid() && row == -1 && column == -1)
        return d->dropOnItem(parent, stream);

    if (row == -1)
        row = rowCount(parent);

    // otherwise insert new rows for the data
    return decodeData(row, column, parent, stream);
}

bool QAbstractListModel::dropMimeData(const QMimeData *data, Qt::DropAction action,
                                      int row, int column, const TzModelIndex &parent)
{
    TZ_D(TzAbstractItemModel);
    if (!data || !(action == Qt::CopyAction || action == Qt::MoveAction))
        return false;

    QStringList types = mimeTypes();
    if (types.isEmpty())
        return false;
    QString format = types.at(0);
    if (!data->hasFormat(format))
        return false;

    std::string encoded = data->data(format);
    QDataStream stream(&encoded, QDataStream::ReadOnly);

    // if the drop is on an item, replace the data in the items
    if (parent.isValid() && row == -1 && column == -1)
        return d->dropOnItem(parent, stream);

    if (row == -1)
        row = rowCount(parent);

    // otherwise insert new rows for the data
    return decodeData(row, column, parent, stream);
}

void TzAbstractItemModelPrivate::Persistent::insertMultiAtEnd(const TzModelIndex& key, TzPersistentModelIndexData *data)
{
    auto newIt = indexes.insert(key, data);
    auto it = newIt;
    ++it;
    while (it != indexes.end() && it.key() == key) {
        qSwap(*newIt,*it);
        newIt = it;
        ++it;
    }
}
