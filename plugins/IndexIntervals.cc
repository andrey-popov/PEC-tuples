#include "IndexIntervals.h"

#include <algorithm>
#include <limits>
#include <sstream>
#include <stdexcept>


IndexIntervals::IterableIndices::IterableIndices(
  IndexIntervals const &parent, index_t min_, index_t max_)
{
    auto const &intervals = parent.intervals;

    if (intervals.empty() or min_ > max_)
    {
        min = 0;
        max = -1;

        beginIntervalIt = intervals.begin();
    }
    else
    {
        // Shrink the overall interval given by min_ and max_ if it's larger than the full span of
        // indices in intervals
        min = std::max(intervals.front().first, min_);
        max = std::min(intervals.back().second, max_);

        beginIntervalIt = parent.FindInterval(min);
    }
}


IndexIntervals::IndexIntervals(std::vector<index_t> edges)
{
    if (edges.size() == 1 and edges[0] == -1)
    {
        // A special notation to select all weights
        intervals.emplace_back(std::make_pair(
          std::numeric_limits<index_t>::min(), std::numeric_limits<index_t>::max()));
    }
    else
    {
        if (edges.size() % 2 != 0)
        {
            std::ostringstream message;
            message << "An odd number of indices (" << edges.size() <<
              ") cannot specify a set of intervals.";
            throw std::runtime_error(message.str());
        }

        for (unsigned i = 0; i < edges.size(); i += 2)
        {
            int const first = edges[i];
            int const last = edges[i + 1];

            if (first > last)
            {
                std::ostringstream message;
                message << "Pair of indices [" << first << ", " << last <<
                  "] is not ordered.";
                throw std::runtime_error(message.str());
            }

            intervals.emplace_back(std::make_pair(first, last));
        }

        // Make sure the vector of intervals is sorted
        std::sort(intervals.begin(), intervals.end(),
          [](auto const &lhs, auto const &rhs){return (lhs.first < rhs.first);});

        // Make sure there are no overlapping intervals
        for (int i = 0; i < int(intervals.size()) - 1; ++i)
        {
            auto const &r1 = intervals[i];
            auto const &r2 = intervals[i + 1];

            if (r1.second > r2.first)
            {
                std::ostringstream message;
                message << "Overlapping intervals found: [" << r1.first << ", " <<
                  r1.second << "] and [" << r2.first << ", " << r2.second << "].";
                throw std::runtime_error(message.str());
            }
        }
    }
}


unsigned IndexIntervals::NumberIndices(index_t min, index_t max) const
{
    if (Empty())
        return 0;

    min = std::max(min, intervals.front().first);
    max = std::min(max, intervals.back().second);

    unsigned count = 0;
    auto end = ++FindInterval(max);

    for (auto intervalIt = FindInterval(min); intervalIt != end; ++intervalIt)
        count += std::min(intervalIt->second, max) - std::max(intervalIt->first, min) + 1;

    return count;
}


std::vector<IndexIntervals::interval_t>::const_iterator
IndexIntervals::FindInterval(index_t index) const
{
    auto const res = std::lower_bound(intervals.begin(), intervals.end(), index,
      [](auto const &r, index_t const &value){return (r.second < value);});

    if (res == intervals.end())
        return res;
    
    if (res->first <= index)  // Condition for the upper boundary is satistied automatically
        return res;
    else
        return intervals.end();
}

