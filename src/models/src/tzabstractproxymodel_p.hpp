#ifndef TZABSTRACTPROXYMODEL_P_HPP
#define TZABSTRACTPROXYMODEL_P_HPP

#include <loom/tzabstractproxymodel.hpp>
#include <tzabstractitemmodel_p.hpp>

class TzAbstractProxyModelPrivate : public TzAbstractItemModelPrivate
{
    TZ_DECLARE_PUBLIC(TzAbstractProxyModel)
public:
    TzAbstractProxyModelPrivate()
        : TzAbstractItemModelPrivate()
        , sourceHadZeroRows(false)
        , sourceHadZeroColumns(false)
        , updateVerticalHeader(false)
        , updateHorizontalHeader(false)
    {}
    void setModelForwarder(TzAbstractItemModel *sourceModel)
    { q_func()->setSourceModel(sourceModel); }
    void modelChangedForwarder()
    { q_func()->sourceModelChanged(); }
    TzAbstractItemModel *getModelForwarder() const
    { return q_func()->sourceModel(); }

    // Q_OBJECT_COMPAT_PROPERTY_WITH_ARGS(TzAbstractProxyModelPrivate, TzAbstractItemModel *, model,
    //                                    &TzAbstractProxyModelPrivate::setModelForwarder,
    //                                    &TzAbstractProxyModelPrivate::modelChangedForwarder,
    //                                    &TzAbstractProxyModelPrivate::getModelForwarder, nullptr)
    virtual void _q_sourceModelDestroyed();
    void _q_sourceModelRowsAboutToBeInserted(const TzModelIndex &parent, int first, int last);
    void _q_sourceModelRowsInserted(const TzModelIndex &parent, int first, int last);
    void _q_sourceModelRowsRemoved(const TzModelIndex &parent, int first, int last);
    void _q_sourceModelColumnsAboutToBeInserted(const TzModelIndex &parent, int first, int last);
    void _q_sourceModelColumnsInserted(const TzModelIndex &parent, int first, int last);
    void _q_sourceModelColumnsRemoved(const TzModelIndex &parent, int first, int last);

    void mapDropCoordinatesToSource(int row, int column, const TzModelIndex &parent,
                                    int *sourceRow, int *sourceColumn, TzModelIndex *sourceParent) const;

    void scheduleHeaderUpdate(TzOrientation orientation);
    void emitHeaderDataChanged();

    unsigned int sourceHadZeroRows : 1;
    unsigned int sourceHadZeroColumns : 1;
    unsigned int updateVerticalHeader : 1;
    unsigned int updateHorizontalHeader : 1;
};

#endif // TZABSTRACTPROXYMODEL_P_HPP
