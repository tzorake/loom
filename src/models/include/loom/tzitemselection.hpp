#ifndef TZITEMSELECTION_HPP
#define TZITEMSELECTION_HPP

#include <loom/tzabstractitemmodel.hpp>

// ---------------------------------------------------------------------------
// TzItemSelectionRange — contiguous rectangular selection within one parent.
// ---------------------------------------------------------------------------
class TzItemSelectionRange
{
public:
    TzItemSelectionRange() = default;

    TzItemSelectionRange(const TzModelIndex &topLeft, const TzModelIndex &bottomRight)
        : m_topLeft(topLeft), m_bottomRight(bottomRight) {}

    // Single-cell convenience constructor
    explicit TzItemSelectionRange(const TzModelIndex &index)
        : m_topLeft(index), m_bottomRight(index) {}

    TzModelIndex topLeft()     const { return m_topLeft; }
    TzModelIndex bottomRight() const { return m_bottomRight; }

    bool isValid() const { return m_topLeft.isValid() && m_bottomRight.isValid(); }

    TzModelIndexList indexes() const
    {
        TzModelIndexList result;
        if (!isValid())
            return result;
        const TzAbstractItemModel *m = m_topLeft.model();
        if (!m || m != m_bottomRight.model())
            return result;
        for (int r = m_topLeft.row(); r <= m_bottomRight.row(); ++r)
            for (int c = m_topLeft.column(); c <= m_bottomRight.column(); ++c)
                result.push_back(m->index(r, c, m_topLeft.parent()));
        return result;
    }

private:
    TzModelIndex m_topLeft;
    TzModelIndex m_bottomRight;
};

// ---------------------------------------------------------------------------
// TzItemSelection — ordered list of selection ranges.
// ---------------------------------------------------------------------------
class TzItemSelection
{
public:
    TzItemSelection() = default;

    void select(const TzModelIndex &topLeft, const TzModelIndex &bottomRight)
    {
        m_ranges.emplace_back(topLeft, bottomRight);
    }

    TzItemSelection &operator<<(const TzItemSelectionRange &range)
    {
        m_ranges.push_back(range);
        return *this;
    }

    TzModelIndexList indexes() const
    {
        TzModelIndexList result;
        for (const auto &r : m_ranges) {
            auto idx = r.indexes();
            result.insert(result.end(), idx.begin(), idx.end());
        }
        return result;
    }

    bool isEmpty() const { return m_ranges.empty(); }
    int  size()    const { return static_cast<int>(m_ranges.size()); }

    const TzItemSelectionRange &at(int i) const { return m_ranges.at(i); }

private:
    std::vector<TzItemSelectionRange> m_ranges;
};

#endif // TZITEMSELECTION_HPP
