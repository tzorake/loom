#pragma once
#include <loom/tzabstractitemmodel.hpp>
#include <loom/tzclasshelpermacros.hpp>

class TzPersistentTzModelIndexData
{
public:
    TzModelIndex index;
    int ref;

    TzPersistentTzModelIndexData() : ref(1) {}
    explicit TzPersistentTzModelIndexData(const TzModelIndex& idx) : index(idx), ref(1) {}

    static TzPersistentTzModelIndexData* create(const TzModelIndex& index);
    static void destroy(TzPersistentTzModelIndexData* data);
};

class TzAbstractItemModelPrivate
{
    TZ_DECLARE_PUBLIC(TzAbstractItemModel)
public:
    TzAbstractItemModelPrivate();
    virtual ~TzAbstractItemModelPrivate();
    
    TzAbstractItemModel* q_ptr = nullptr;
    
    struct Persistent {
        std::unordered_multimap<TzModelIndex, TzPersistentTzModelIndexData*> indexes;
        std::list<std::vector<TzPersistentTzModelIndexData*>> moved;
        std::list<std::vector<TzPersistentTzModelIndexData*>> invalidated;
        
        void insertMultiAtEnd(const TzModelIndex& key, TzPersistentTzModelIndexData* data) {
            indexes.emplace(key, data);
        }
    } persistent;
    
    struct Change {
        TzModelIndex parent;
        int first;
        int last;
        bool needsAdjust;
        
        Change() : first(-1), last(-1), needsAdjust(false) {}
        Change(const TzModelIndex& p, int f, int l) : parent(p), first(f), last(l), needsAdjust(false) {}
    };
    
    Change insertRowsChange;
    Change removeRowsChange;
    Change insertColumnsChange;
    Change removeColumnsChange;
    
    void removePersistentIndexData(TzPersistentTzModelIndexData* data);
    void invalidatePersistentIndexes();
    void invalidatePersistentIndex(const TzModelIndex& index);
    
    void rowsAboutToBeInserted(const TzModelIndex& parent, int first, int last);
    void rowsInserted(const TzModelIndex& parent, int first, int last);
    void rowsAboutToBeRemoved(const TzModelIndex& parent, int first, int last);
    void rowsRemoved(const TzModelIndex& parent, int first, int last);
    
    void columnsAboutToBeInserted(const TzModelIndex& parent, int first, int last);
    void columnsInserted(const TzModelIndex& parent, int first, int last);
    void columnsAboutToBeRemoved(const TzModelIndex& parent, int first, int last);
    void columnsRemoved(const TzModelIndex& parent, int first, int last);
    
    void itemsAboutToBeMoved(const TzModelIndex& sourceParent, int sourceFirst, int sourceLast,
                            const TzModelIndex& destinationParent, int destinationChild,
                            TzAbstractItemModel::Orientation orientation);
    void itemsMoved(const TzModelIndex& sourceParent, int sourceFirst, int sourceLast,
                   const TzModelIndex& destinationParent, int destinationChild,
                   TzAbstractItemModel::Orientation orientation);
    
    void movePersistentIndexes(const std::vector<TzPersistentTzModelIndexData*>& indexes, int change,
                              const TzModelIndex& parent, TzAbstractItemModel::Orientation orientation);
    
    bool allowMove(const TzModelIndex& srcParent, int start, int end, const TzModelIndex& destinationParent,
                  int destinationStart, TzAbstractItemModel::Orientation orientation);
};
