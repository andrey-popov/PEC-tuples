#pragma once

#include <boost/iterator/iterator_facade.hpp>

#include <utility>
#include <vector>


/**
 * \brief A set of closed intervals of indices
 *
 * Allows to test whether a given index is contained within one of the included intervals (using
 * method \ref Contain). Provides means to iterate over all indices in all intervals (method
 * \ref GetIndices).
 */
class IndexIntervals
{
public:
    /// Signed integer type that represents index
    using index_t = int;

    /// Closed interval
    using interval_t = std::pair<index_t, index_t>;

    /// Auxiliary class that implements a forward iterator for indices
    class IndexIt: public boost::iterator_facade<
      IndexIt, index_t const, boost::forward_traversal_tag, index_t const &, int>
    {
    public:
        IndexIt() = default;

        IndexIt(index_t index_, std::vector<interval_t>::const_iterator curIntervalIt_):
            index{index_}, curIntervalIt{curIntervalIt_}
        {}

    private:
        friend class boost::iterator_core_access;

        reference dereference() const
        {
            // Lazily move to the next interval
            if (index > curIntervalIt->second)
            {
                ++curIntervalIt;
                index = curIntervalIt->first;
            }

            return index;
        }

        bool equal(IndexIt const &other) const
        {
            return index == other.index;
        }

        void increment()
        {
            ++index;

            // It is possible that at this point the index has stepped outside of the current
            // interval. This will be checked in method dereference in order to avoid dereferencing
            // of the post-end interval iterator here.
        }

        mutable index_t index;
        mutable std::vector<interval_t>::const_iterator curIntervalIt;
    };

    /// Auxiliary class to implement iteration over indices
    class IterableIndices
    {
    public:
        IterableIndices(IndexIntervals const &parent, index_t min, index_t max);

        IndexIt begin() const
        {
            return {min, beginIntervalIt};
        }

        IndexIt end() const
        {
            return {max + 1, decltype(beginIntervalIt)()};
        }

    private:
        /// Only indices within this range will be considered
        index_t min, max;

        /// First interval (potentiall partly) included in the full considered range
        std::vector<interval_t>::const_iterator beginIntervalIt;
    };

public:
    /**
     * \brief Constructor from edges of intervals
     *
     * Except for the special case below, the size of the vector of edges must be an even number.
     * Elements in the vector are interpreted as pairs of indices that define close intervals.
     * Different intervals do not have to be sorted, but they may not overlap.
     *
     * If the vector contains a single value (-1), a single interval that convers all possible
     * values of indices is created.
     */
    IndexIntervals(std::vector<index_t> edges);

    /// Checks if given index is contained in one of the intervals
    bool Contain(index_t index) const
    {
        return (FindInterval(index) != intervals.end());
    }

    /// Checks there are any intervals
    bool Empty() const
    {
        return intervals.empty();
    }

    /// Returns vector of all intervals of indices
    std::vector<interval_t> const &Get() const
    {
        return intervals;
    }

    /**
     * \brief Returns an iterable object to visit all indices in given range
     *
     * Returned object provides forward iterators to loop over all indices in all intervals,
     * restricting them to the given range (boundaries are included). The indices are visited in an
     * increasing order.
     */
    IterableIndices GetIndices(index_t min, index_t max) const
    {
        return {*this, min, max};
    }

    /**
     * \brief Returns the number of indices, restricting them to given range
     *
     * The boundaries of the range are included.
     */
    unsigned NumberIndices(index_t min, index_t max) const;

private:
    /// Returns iterator to the interval that contains given index
    std::vector<interval_t>::const_iterator FindInterval(index_t index) const;

    /**
     * \brief Vector of closed intervals
     *
     * Each included interval is guaranteed to contain at least one index. The intervals do not
     * overlap (although can be adjacent). They are sorted in the increasing order.
     */
    std::vector<interval_t> intervals;
};

