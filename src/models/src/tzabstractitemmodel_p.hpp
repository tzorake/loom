#ifndef TZABSTRACTITEMMODEL_P_HPP
#define TZABSTRACTITEMMODEL_P_HPP

#include <loom/tzabstractitemmodel.hpp>
#include <loom/tzeventemitter.hpp>

class TzPersistentModelIndexData
{
public:
    TzPersistentModelIndexData() {}
    TzPersistentModelIndexData(const TzModelIndex &idx) : index(idx) {}
    TzModelIndex index;
    QAtomicInt ref;
    static TzPersistentModelIndexData *create(const TzModelIndex &index);
    static void destroy(TzPersistentModelIndexData *data);
};

class TzAbstractItemModelPrivate
{
    TZ_DECLARE_PUBLIC(TzAbstractItemModel)
public:
    TzAbstractItemModelPrivate();
    ~TzAbstractItemModelPrivate();

    static const TzAbstractItemModelPrivate *get(const TzAbstractItemModel *model) { return model->d_func(); }

    void removePersistentIndexData(TzPersistentModelIndexData *data);
    void movePersistentIndexes(const std::vector<TzPersistentModelIndexData *> &indexes, int change, const TzModelIndex &parent, TzOrientation orientation);
    void rowsAboutToBeInserted(const TzModelIndex &parent, int first, int last);
    void rowsInserted(const TzModelIndex &parent, int first, int last);
    void rowsAboutToBeRemoved(const TzModelIndex &parent, int first, int last);
    void rowsRemoved(const TzModelIndex &parent, int first, int last);
    void columnsAboutToBeInserted(const TzModelIndex &parent, int first, int last);
    void columnsInserted(const TzModelIndex &parent, int first, int last);
    void columnsAboutToBeRemoved(const TzModelIndex &parent, int first, int last);
    void columnsRemoved(const TzModelIndex &parent, int first, int last);
    static TzAbstractItemModel *staticEmptyModel();

    void itemsAboutToBeMoved(const TzModelIndex &srcParent, int srcFirst, int srcLast, const TzModelIndex &destinationParent, int destinationChild, TzOrientation);
    void itemsMoved(const TzModelIndex &srcParent, int srcFirst, int srcLast, const TzModelIndex &destinationParent, int destinationChild, TzOrientation orientation);
    bool allowMove(const TzModelIndex &srcParent, int srcFirst, int srcLast, const TzModelIndex &destinationParent, int destinationChild, TzOrientation orientation);

    virtual void executePendingOperations() const;

    inline TzModelIndex createIndex(int row, int column, void *data = nullptr) const
    { return q_func()->createIndex(row, column, data); }

    inline TzModelIndex createIndex(int row, int column, int id) const
    { return q_func()->createIndex(row, column, id); }

    inline bool indexValid(const TzModelIndex &index) const
    { return (index.row() >= 0) && (index.column() >= 0) && (index.model() == q_func()); }

    void invalidatePersistentIndexes();
    void invalidatePersistentIndex(const TzModelIndex &index);

    struct Change {
        constexpr Change() : parent(), first(-1), last(-1), needsAdjust(false) {}
        constexpr Change(const TzModelIndex &p, int f, int l) : parent(p), first(f), last(l), needsAdjust(false) {}

        TzModelIndex parent;
        int first, last;

        // In cases such as this:
        // - A
        // - B
        // - C
        // - - D
        // - - E
        // - - F
        //
        // If B is moved to above E, C is the source parent in the signal and its row is 2. When the move is
        // completed however, C is at row 1 and there is no row 2 at the same level in the model at all.
        // The TzModelIndex is adjusted to correct that in those cases before reporting it though the
        // rowsMoved signal.
        bool needsAdjust;

        constexpr bool isValid() const { return first >= 0 && last >= 0; }
    };
    std::stack<Change> changes;

    struct Persistent {
        Persistent() {}
        std::unordered_multimap<TzModelIndex, TzPersistentModelIndexData *> indexes;
        std::stack<std::vector<TzPersistentModelIndexData *>> moved;
        std::stack<std::vector<TzPersistentModelIndexData *>> invalidated;
        void insertMultiAtEnd(const TzModelIndex& key, TzPersistentModelIndexData *data);
    } persistent;

    bool resetting = false;

    static const std::unordered_map<int, std::string> &defaultRoleNames();

    TzEventEmitter emitter;
};

#endif // TZABSTRACTITEMMODEL_P_HPP
