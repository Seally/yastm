#pragma once

#include <functional>
#include <iterator>
#include <unordered_set>

template <
    typename iterator,
    typename Hash =
        std::hash<typename std::iterator_traits<iterator>::value_type>,
    typename KeyEqual =
        std::equal_to<typename std::iterator_traits<iterator>::value_type>>
inline bool areAllUnique(iterator begin, iterator end)
{
    using T = std::iterator_traits<iterator>::value_type;

    // Use an unordered set to disambiguate between elements.
    std::unordered_set<std::reference_wrapper<const T>, Hash, KeyEqual> uniques;

    std::size_t count = 0;

    for (auto it = begin; it != end; ++it) {
        uniques.emplace(*it);
        ++count;
    }

    return uniques.size() == count;
}
