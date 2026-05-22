#ifndef TZABSTRACTPROXYMODEL_P_HPP
#define TZABSTRACTPROXYMODEL_P_HPP

#include <loom/tzabstractproxymodel.hpp>
#include <tzabstractitemmodel_p.hpp>

#include <loom/tzeventlistener.hpp>
#include <vector>

class TzAbstractProxyModelPrivate : public TzAbstractItemModelPrivate
{
    TZ_DECLARE_PUBLIC(TzAbstractProxyModel)
public:
    TzAbstractProxyModelPrivate()
        : TzAbstractItemModelPrivate()
        , model(nullptr)
        , sourceHadZeroRows(false)
        , sourceHadZeroColumns(false)
        , updateVerticalHeader(false)
        , updateHorizontalHeader(false)
    {}

    // The wrapped source model (never null after construction; points to
    // staticEmptyModel() when no real source has been set).
    TzAbstractItemModel *model;

    // Listeners on the current source model's emitter — disconnected when the
    // source model is replaced or the proxy is destroyed.
    std::vector<TzEventListener> sourceListeners;

    void setModelForwarder(TzAbstractItemModel *sourceModel)
    { q_func()->setSourceModel(sourceModel); }
    void modelChangedForwarder()
    { q_func()->sourceModelChanged(); }
    TzAbstractItemModel *getModelForwarder() const
    { return q_func()->sourceModel(); }

    virtual void _q_sourceModelDestroyed();
    void _q_sourceModelRowsAboutToBeInserted(const TzModelIndex &parent, int first, int last);
    void _q_sourceModelRowsInserted(const TzModelIndex &parent, int first, int last);
    void _q_sourceModelRowsRemoved(const TzModelIndex &parent, int first, int last);
    void _q_sourceModelColumnsAboutToBeInserted(const TzModelIndex &parent, int first, int last);
    void _q_sourceModelColumnsInserted(const TzModelIndex &parent, int first, int last);
    void _q_sourceModelColumnsRemoved(const TzModelIndex &parent, int first, int last);

    void scheduleHeaderUpdate(TzOrientation orientation);
    void emitHeaderDataChanged();

    unsigned int sourceHadZeroRows : 1;
    unsigned int sourceHadZeroColumns : 1;
    unsigned int updateVerticalHeader : 1;
    unsigned int updateHorizontalHeader : 1;
};

#endif // TZABSTRACTPROXYMODEL_P_HPP
