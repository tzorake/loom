#include <loom/tzabstractproxymodel.hpp>
#include <tzabstractproxymodel_p.hpp>
#include <loom/tzitemselectionmodel.hpp>

void TzAbstractProxyModelPrivate::_q_sourceModelDestroyed()
{
    invalidatePersistentIndexes();
    model = TzAbstractItemModelPrivate::staticEmptyModel();
}

void TzAbstractProxyModelPrivate::emitHeaderDataChanged()
{
    TZ_Q(TzAbstractProxyModel);

    if (updateHorizontalHeader) {
        if (auto columnCount = q->columnCount(); columnCount > 0)
            emit q->headerDataChanged(TzOrientation::Horizontal, 0, columnCount - 1);
    }

    if (updateVerticalHeader) {
        if (auto rowCount = q->rowCount(); rowCount > 0)
            emit q->headerDataChanged(TzOrientation::Vertical, 0, rowCount - 1);
    }

    updateHorizontalHeader = false;
    updateVerticalHeader = false;
}

void TzAbstractProxyModelPrivate::scheduleHeaderUpdate(TzOrientation orientation)
{
    const bool isUpdateScheduled = updateHorizontalHeader || updateVerticalHeader;

    if (orientation == TzOrientation::Horizontal && !updateHorizontalHeader)
        updateHorizontalHeader = true;
    else if (orientation == TzOrientation::Vertical && !updateVerticalHeader)
        updateVerticalHeader = true;
    else
        return;

    if (!isUpdateScheduled) {
        TZ_Q(TzAbstractProxyModel);
        QMetaObject::invokeMethod(q, [this]() { emitHeaderDataChanged(); }, Qt::QueuedConnection);
    }
}

void TzAbstractProxyModelPrivate::_q_sourceModelRowsAboutToBeInserted(const TzModelIndex &parent, int, int)
{
    if (parent.isValid())
        return;
    sourceHadZeroRows = model->rowCount() == 0;
}

void TzAbstractProxyModelPrivate::_q_sourceModelRowsInserted(const TzModelIndex &parent, int, int)
{
    if (parent.isValid())
        return;
    if (sourceHadZeroRows)
        scheduleHeaderUpdate(TzOrientation::Horizontal);
}

void TzAbstractProxyModelPrivate::_q_sourceModelRowsRemoved(const TzModelIndex &parent, int, int)
{
    if (parent.isValid())
        return;
    if (model->rowCount() == 0)
        scheduleHeaderUpdate(TzOrientation::Horizontal);
}

void TzAbstractProxyModelPrivate::_q_sourceModelColumnsAboutToBeInserted(const TzModelIndex &parent, int, int)
{
    if (parent.isValid())
        return;
    sourceHadZeroColumns = model->columnCount() == 0;
}

void TzAbstractProxyModelPrivate::_q_sourceModelColumnsInserted(const TzModelIndex &parent, int, int)
{
    if (parent.isValid())
        return;
    if (sourceHadZeroColumns)
        scheduleHeaderUpdate(TzOrientation::Vertical);
}

void TzAbstractProxyModelPrivate::_q_sourceModelColumnsRemoved(const TzModelIndex &parent, int, int)
{
    if (parent.isValid())
        return;
    if (model->columnCount() == 0)
        scheduleHeaderUpdate(TzOrientation::Vertical);
}

TzAbstractProxyModel::TzAbstractProxyModel()
    :TzAbstractItemModel(*new TzAbstractProxyModelPrivate)
{
    setSourceModel(TzAbstractItemModelPrivate::staticEmptyModel());
}

TzAbstractProxyModel::TzAbstractProxyModel(TzAbstractProxyModelPrivate &dd)
    : TzAbstractItemModel(dd)
{
    setSourceModel(TzAbstractItemModelPrivate::staticEmptyModel());
}

TzAbstractProxyModel::~TzAbstractProxyModel()
{
}

void TzAbstractProxyModel::setSourceModel(TzAbstractItemModel *sourceModel)
{
    TZ_D(TzAbstractProxyModel);
    d->model.removeBindingUnlessInWrapper();
    // Special case to handle nullptr models. Otherwise we will have unwanted
    // notifications.
    const TzAbstractItemModel *currentModel = d->model.valueBypassingBindings();
    if (!sourceModel && currentModel == TzAbstractItemModelPrivate::staticEmptyModel())
        return;
    static const struct {
        const char *signalName;
        const char *slotName;
    } connectionTable[] = {
        // clang-format off
        { SIGNAL(destroyed()), SLOT(_q_sourceModelDestroyed()) },
        { SIGNAL(rowsAboutToBeInserted(TzModelIndex,int,int)), SLOT(_q_sourceModelRowsAboutToBeInserted(TzModelIndex,int,int)) },
        { SIGNAL(rowsInserted(TzModelIndex,int,int)), SLOT(_q_sourceModelRowsInserted(TzModelIndex,int,int)) },
        { SIGNAL(rowsRemoved(TzModelIndex,int,int)), SLOT(_q_sourceModelRowsRemoved(TzModelIndex,int,int)) },
        { SIGNAL(columnsAboutToBeInserted(TzModelIndex,int,int)), SLOT(_q_sourceModelColumnsAboutToBeInserted(TzModelIndex,int,int)) },
        { SIGNAL(columnsInserted(TzModelIndex,int,int)), SLOT(_q_sourceModelColumnsInserted(TzModelIndex,int,int)) },
        { SIGNAL(columnsRemoved(TzModelIndex,int,int)), SLOT(_q_sourceModelColumnsRemoved(TzModelIndex,int,int)) }
        // clang-format on
    };

    if (sourceModel != currentModel) {
        if (currentModel) {
            for (const auto &c : connectionTable)
                disconnect(currentModel, c.signalName, this, c.slotName);
        }

        if (sourceModel) {
            d->model.setValueBypassingBindings(sourceModel);
            for (const auto &c : connectionTable)
                connect(sourceModel, c.signalName, this, c.slotName);
        } else {
            d->model.setValueBypassingBindings(TzAbstractItemModelPrivate::staticEmptyModel());
        }
        d->model.notify();
    }
}

TzAbstractItemModel *TzAbstractProxyModel::sourceModel() const
{
    TZ_D(const TzAbstractProxyModel);
    if (d->model == TzAbstractItemModelPrivate::staticEmptyModel())
        return nullptr;
    return d->model;
}

bool TzAbstractProxyModel::submit()
{
    TZ_D(TzAbstractProxyModel);
    return d->model->submit();
}

void TzAbstractProxyModel::revert()
{
    TZ_D(TzAbstractProxyModel);
    d->model->revert();
}

TzItemSelection TzAbstractProxyModel::mapSelectionToSource(const TzItemSelection &proxySelection) const
{
    TzModelIndexList proxyIndexes = proxySelection.indexes();
    TzItemSelection sourceSelection;
    for (int i = 0; i < proxyIndexes.size(); ++i) {
        const TzModelIndex proxyIdx = mapToSource(proxyIndexes.at(i));
        if (!proxyIdx.isValid())
            continue;
        sourceSelection << TzItemSelectionRange(proxyIdx);
    }
    return sourceSelection;
}

TzItemSelection TzAbstractProxyModel::mapSelectionFromSource(const TzItemSelection &sourceSelection) const
{
    TzModelIndexList sourceIndexes = sourceSelection.indexes();
    TzItemSelection proxySelection;
    for (int i = 0; i < sourceIndexes.size(); ++i) {
        const TzModelIndex srcIdx = mapFromSource(sourceIndexes.at(i));
        if (!srcIdx.isValid())
            continue;
        proxySelection << TzItemSelectionRange(srcIdx);
    }
    return proxySelection;
}

std::any TzAbstractProxyModel::data(const TzModelIndex &proxyIndex, int role) const
{
    TZ_D(const TzAbstractProxyModel);
    return d->model->data(mapToSource(proxyIndex), role);
}

std::any TzAbstractProxyModel::headerData(int section, TzOrientation orientation, int role) const
{
    TZ_D(const TzAbstractProxyModel);
    int sourceSection = section;
    if (orientation == TzOrientation::Horizontal) {
        if (rowCount() > 0) {
            const TzModelIndex proxyIndex = index(0, section);
            sourceSection = mapToSource(proxyIndex).column();
        }
    } else {
        if (columnCount() > 0) {
            const TzModelIndex proxyIndex = index(section, 0);
            sourceSection = mapToSource(proxyIndex).row();
        }
    }
    return d->model->headerData(sourceSection, orientation, role);
}

TzItemFlags TzAbstractProxyModel::flags(const TzModelIndex &index) const
{
    TZ_D(const TzAbstractProxyModel);
    return d->model->flags(mapToSource(index));
}

bool TzAbstractProxyModel::setData(const TzModelIndex &index, const std::any &value, int role)
{
    TZ_D(TzAbstractProxyModel);
    return d->model->setData(mapToSource(index), value, role);
}

bool TzAbstractProxyModel::setHeaderData(int section, TzOrientation orientation, const std::any &value, int role)
{
    TZ_D(TzAbstractProxyModel);
    int sourceSection;
    if (orientation == TzOrientation::Horizontal) {
        const TzModelIndex proxyIndex = index(0, section);
        sourceSection = mapToSource(proxyIndex).column();
    } else {
        const TzModelIndex proxyIndex = index(section, 0);
        sourceSection = mapToSource(proxyIndex).row();
    }
    return d->model->setHeaderData(sourceSection, orientation, value, role);
}

TzModelIndex TzAbstractProxyModel::buddy(const TzModelIndex &index) const
{
    TZ_D(const TzAbstractProxyModel);
    return mapFromSource(d->model->buddy(mapToSource(index)));
}

bool TzAbstractProxyModel::canFetchMore(const TzModelIndex &parent) const
{
    TZ_D(const TzAbstractProxyModel);
    return d->model->canFetchMore(mapToSource(parent));
}

void TzAbstractProxyModel::fetchMore(const TzModelIndex &parent)
{
    TZ_D(TzAbstractProxyModel);
    d->model->fetchMore(mapToSource(parent));
}

void TzAbstractProxyModel::sort(int column, Qt::SortOrder order)
{
    TZ_D(TzAbstractProxyModel);
    d->model->sort(column, order);
}

bool TzAbstractProxyModel::hasChildren(const TzModelIndex &parent) const
{
    TZ_D(const TzAbstractProxyModel);
    return d->model->hasChildren(mapToSource(parent));
}

TzModelIndex TzAbstractProxyModel::sibling(int row, int column, const TzModelIndex &idx) const
{
    return index(row, column, idx.parent());
}

std::unordered_map<int, std::string> TzAbstractProxyModel::roleNames() const
{
  TZ_D(const TzAbstractProxyModel);
  return d->model->roleNames();
}

TzModelIndex TzAbstractProxyModel::createSourceIndex(int row, int col, void *internalPtr) const
{
    if (sourceModel())
        return sourceModel()->createIndex(row, col, internalPtr);
    return TzModelIndex();
}
