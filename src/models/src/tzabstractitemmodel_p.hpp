// #pragma once
// #include <loom/tzabstractitemmodel.hpp>
// #include <loom/tzclasshelpermacros.hpp>

// class TzPersistentTzModelIndexData
// {
// public:
//     TzModelIndex index;
//     int ref;

//     TzPersistentTzModelIndexData()
//         : ref(1)
//     {}
//     explicit TzPersistentTzModelIndexData(const TzModelIndex &idx)
//         : index(idx)
//         , ref(1)
//     {}

//     static TzPersistentTzModelIndexData *create(const TzModelIndex &index);
//     static void destroy(TzPersistentTzModelIndexData *data);
// };

// class TzAbstractItemModelPrivate
// {
//     TZ_DECLARE_PUBLIC(TzAbstractItemModel)
// public:
//     TzAbstractItemModelPrivate();
//     virtual ~TzAbstractItemModelPrivate();

//     TzAbstractItemModel *q_ptr = nullptr;

//     struct Persistent
//     {
//         std::unordered_multimap<TzModelIndex, TzPersistentTzModelIndexData *> indexes;
//         std::list<std::vector<TzPersistentTzModelIndexData *>> moved;
//         std::list<std::vector<TzPersistentTzModelIndexData *>> invalidated;

//         void insertMultiAtEnd(const TzModelIndex &key, TzPersistentTzModelIndexData *data)
//         {
//             indexes.emplace(key, data);
//         }
//     } persistent;

//     struct Change
//     {
//         TzModelIndex parent;
//         int first;
//         int last;
//         bool needsAdjust;

//         Change()
//             : first(-1)
//             , last(-1)
//             , needsAdjust(false)
//         {}
//         Change(const TzModelIndex &p, int f, int l)
//             : parent(p)
//             , first(f)
//             , last(l)
//             , needsAdjust(false)
//         {}
//     };

//     Change insertRowsChange;
//     Change removeRowsChange;
//     Change insertColumnsChange;
//     Change removeColumnsChange;

//     void removePersistentIndexData(TzPersistentTzModelIndexData *data);
//     void invalidatePersistentIndexes();
//     void invalidatePersistentIndex(const TzModelIndex &index);

//     void rowsAboutToBeInserted(const TzModelIndex &parent, int first, int last);
//     void rowsInserted(const TzModelIndex &parent, int first, int last);
//     void rowsAboutToBeRemoved(const TzModelIndex &parent, int first, int last);
//     void rowsRemoved(const TzModelIndex &parent, int first, int last);

//     void columnsAboutToBeInserted(const TzModelIndex &parent, int first, int last);
//     void columnsInserted(const TzModelIndex &parent, int first, int last);
//     void columnsAboutToBeRemoved(const TzModelIndex &parent, int first, int last);
//     void columnsRemoved(const TzModelIndex &parent, int first, int last);

//     void itemsAboutToBeMoved(const TzModelIndex &sourceParent, int sourceFirst, int sourceLast,
//                              const TzModelIndex &destinationParent, int destinationChild,
//                              TzAbstractItemModel::Orientation orientation);
//     void itemsMoved(const TzModelIndex &sourceParent, int sourceFirst, int sourceLast,
//                     const TzModelIndex &destinationParent, int destinationChild,
//                     TzAbstractItemModel::Orientation orientation);

//     void movePersistentIndexes(const std::vector<TzPersistentTzModelIndexData *> &indexes,
//                                int change, const TzModelIndex &parent,
//                                TzAbstractItemModel::Orientation orientation);

//     bool allowMove(const TzModelIndex &srcParent, int start, int end,
//                    const TzModelIndex &destinationParent, int destinationStart,
//                    TzAbstractItemModel::Orientation orientation);
// };

class TzPersistentModelIndexData
{
public:
    TzPersistentModelIndexData() {}
    TzPersistentModelIndexData(const TzModelIndex &idx) : index(idx) {}
    TzModelIndex index;
    int ref;
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
    void movePersistentIndexes(const std::vector<TzPersistentModelIndexData *> &indexes, int change, const TzModelIndex &parent,
                               Qt::Orientation orientation);
    void rowsAboutToBeInserted(const TzModelIndex &parent, int first, int last);
    void rowsInserted(const TzModelIndex &parent, int first, int last);
    void rowsAboutToBeRemoved(const TzModelIndex &parent, int first, int last);
    void rowsRemoved(const TzModelIndex &parent, int first, int last);
    void columnsAboutToBeInserted(const TzModelIndex &parent, int first, int last);
    void columnsInserted(const TzModelIndex &parent, int first, int last);
    void columnsAboutToBeRemoved(const TzModelIndex &parent, int first, int last);
    void columnsRemoved(const TzModelIndex &parent, int first, int last);
    static TzAbstractItemModel *staticEmptyModel();

    void itemsAboutToBeMoved(const TzModelIndex &srcParent, int srcFirst, int srcLast, const TzModelIndex &destinationParent, int destinationChild, Qt::Orientation);
    void itemsMoved(const TzModelIndex &srcParent, int srcFirst, int srcLast, const TzModelIndex &destinationParent, int destinationChild, Qt::Orientation orientation);
    bool allowMove(const TzModelIndex &srcParent, int srcFirst, int srcLast, const TzModelIndex &destinationParent, int destinationChild, Qt::Orientation orientation);
    bool dropOnItem(const TzModelIndex &index, QDataStream &stream);

    // ugly hack for QTreeModel, see QTBUG-94546
    virtual void executePendingOperations() const;

    inline TzModelIndex createIndex(int row, int column, void *data = nullptr) const {
        return q_func()->createIndex(row, column, data);
    }

    inline TzModelIndex createIndex(int row, int column, int id) const {
        return q_func()->createIndex(row, column, id);
    }

    inline bool indexValid(const TzModelIndex &index) const {
         return (index.row() >= 0) && (index.column() >= 0) && (index.model() == q_func());
    }

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
        std::stack<std::list<TzPersistentModelIndexData *>> moved;
        std::stack<std::list<TzPersistentModelIndexData *>> invalidated;
        void insertMultiAtEnd(const TzModelIndex& key, TzPersistentModelIndexData *data);
    } persistent;

    bool resetting = false;

    static const std::unordered_map<int, std::string> &defaultRoleNames();
};
