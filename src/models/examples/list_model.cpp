#include <loom/tzabstractlistmodel.hpp>
#include <loom/tzeventemitter.hpp>
#include <any>
#include <print>
#include <string>
#include <vector>

class StringListModel : public TzAbstractListModel
{
public:
    StringListModel() = default;

    explicit StringListModel(std::vector<std::string> items)
        : m_items(std::move(items))
    {}

    int rowCount(const TzModelIndex &parent = TzModelIndex()) const override
    {
        return parent.isValid() ? 0 : static_cast<int>(m_items.size());
    }

    std::any data(const TzModelIndex &index, int role = TzItemDataRole::DisplayRole) const override
    {
        if (!index.isValid() || index.row() >= rowCount())
            return {};
        if (role == TzItemDataRole::DisplayRole || role == TzItemDataRole::EditRole)
            return m_items[static_cast<std::size_t>(index.row())];
        return {};
    }

    bool setData(const TzModelIndex &index, const std::any &value,
                 int role = TzItemDataRole::EditRole) override
    {
        if (!index.isValid() || index.row() >= rowCount())
            return false;
        if (role != TzItemDataRole::EditRole)
            return false;

        m_items[static_cast<std::size_t>(index.row())] = std::any_cast<std::string>(value);
        dataChanged(index, index, {TzItemDataRole::DisplayRole, TzItemDataRole::EditRole});
        return true;
    }

    bool insertRows(int row, int count, const TzModelIndex &parent = TzModelIndex()) override
    {
        if (parent.isValid() || row < 0 || row > rowCount() || count <= 0)
            return false;

        beginInsertRows(parent, row, row + count - 1);
        m_items.insert(m_items.begin() + row, static_cast<std::size_t>(count), {});
        endInsertRows();
        return true;
    }

    bool removeRows(int row, int count, const TzModelIndex &parent = TzModelIndex()) override
    {
        if (parent.isValid() || row < 0 || count <= 0 || row + count > rowCount())
            return false;

        beginRemoveRows(parent, row, row + count - 1);
        m_items.erase(m_items.begin() + row, m_items.begin() + row + count);
        endRemoveRows();
        return true;
    }

    void append(const std::string &value)
    {
        const int row = rowCount();
        insertRows(row, 1);
        setData(index(row, 0), value);
    }

    void insert(int row, const std::string &value)
    {
        insertRows(row, 1);
        setData(index(row, 0), value);
    }

    void remove(int row) { removeRows(row, 1); }

    std::string at(int row) const
    {
        auto d = data(index(row, 0));
        return d.has_value() ? std::any_cast<std::string>(d) : std::string{};
    }

private:
    std::vector<std::string> m_items;
};

int main()
{
    StringListModel model({"Alice", "Bob", "Charlie"});

    auto onInserted = model.events().on("rowsInserted", [&](const TzModelIndex & /*parent*/,
                                                            int first, int last) {
        for (int r = first; r <= last; ++r)
            std::println("  [+] row {} = \"{}\"", r, model.at(r));
    });

    auto onRemoved = model.events().on("rowsRemoved",
                                       [&](const TzModelIndex & /*parent*/, int first, int last) {
                                           std::println("  [-] rows {}..{} removed", first, last);
                                       });

    auto onChanged = model.events().on("dataChanged", [&](const TzModelIndex &topLeft,
                                                          const TzModelIndex &bottomRight,
                                                          const std::vector<int> & /*roles*/) {
        for (int r = topLeft.row(); r <= bottomRight.row(); ++r)
            std::println("  [~] row {} = \"{}\"", r, model.at(r));
    });

    std::println("Initial rows: {}", model.rowCount());
    for (int r = 0; r < model.rowCount(); ++r)
        std::println("  [{}] \"{}\"", r, model.at(r));

    std::println("\nappend(\"Dave\"):");
    model.append("Dave");

    std::println("\ninsert(1, \"Zara\"):");
    model.insert(1, "Zara");

    std::println("\nsetData(row 0, \"Alicia\"):");
    model.setData(model.index(0, 0), std::string("Alicia"));

    std::println("\nremove(row 2):");
    model.remove(2);

    std::println("\nFinal rows: {}", model.rowCount());
    for (int r = 0; r < model.rowCount(); ++r)
        std::println("  [{}] \"{}\"", r, model.at(r));

    return 0;
}
