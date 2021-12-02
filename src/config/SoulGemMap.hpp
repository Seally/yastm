#pragma once

#include <array>
#include <compare>
#include <functional>
#include <vector>
#include <memory>

#include "../global.hpp"
#include "SoulSize.hpp"
#include "ConcreteSoulGemGroup.hpp"
#include "SpecificationError.hpp"

namespace RE {
    class TESDataHandler;
    class TESSoulGem;
} // end namespace RE

class SoulGemGroup;

class SoulGemMap {
public:
    class Iterator;
    using IteratorPair = std::pair<Iterator, Iterator>;

private:
    using SoulGemList = std::vector<RE::TESSoulGem*>;
    using ConcreteSoulGemGroupList =
        std::vector<std::unique_ptr<ConcreteSoulGemGroup>>;
    using FormMap =
        std::unordered_map<SoulGemCapacity, ConcreteSoulGemGroupList>;
    FormMap _soulGemMap;

    friend class Iterator;

public:
    class Transaction {
        std::vector<std::reference_wrapper<const SoulGemGroup>> _groupsToAdd;

        friend class SoulGemMap;

        explicit Transaction() = default;
        Transaction(const Transaction&) = delete;
        Transaction(Transaction&&) = delete;
        Transaction& operator=(const Transaction&) = delete;
        Transaction& operator=(Transaction&&) = delete;

    public:
        void addSoulGemGroup(const SoulGemGroup& group)
        {
            _groupsToAdd.emplace_back(group);
        }
    };

    class Iterator {
    public:
        using iterator_category = std::random_access_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = RE::TESSoulGem;
        using pointer = value_type*;
        using reference = value_type&;

    private:
        SoulSize _containedSoulSize;
        std::size_t _index;
        const ConcreteSoulGemGroupList* _soulGemsAtCapacity;

        explicit Iterator(
            const ConcreteSoulGemGroupList& soulGemsAtCapacity,
            const SoulSize containedSoulSize,
            const std::size_t index)
            : _soulGemsAtCapacity(&soulGemsAtCapacity)
            , _containedSoulSize(containedSoulSize)
            , _index(index)
        {}

        friend class SoulGemMap;

    public:
        Iterator(const Iterator& other) = default;
        Iterator(Iterator&& other) = default;
        Iterator& operator=(const Iterator& other) = default;
        Iterator& operator=(Iterator&& other) = default;

        const ConcreteSoulGemGroup& group() const
        {
            return *_soulGemsAtCapacity->at(_index);
        }

        const SoulSize containedSoulSize() const { return _containedSoulSize; }
        pointer get() const { return group().at(_containedSoulSize); }

        reference operator*() const { return *group().at(_containedSoulSize); }
        pointer operator->() const { return get(); }

        Iterator& operator++()
        {
            ++_index;
            return *this;
        }

        Iterator operator++(int)
        {
            Iterator tmp(*this);
            ++_index;
            return tmp;
        }

        Iterator& operator--()
        {
            --_index;
            return *this;
        }

        Iterator operator--(int)
        {
            Iterator tmp = *this;
            --_index;
            return tmp;
        }

        Iterator& operator+=(const difference_type n)
        {
            _index += n;
            return *this;
        }

        friend Iterator operator+(const Iterator& it, const difference_type n)
        {
            Iterator tmp = it;
            tmp._index += n;
            return tmp;
        }

        Iterator& operator-=(const difference_type n)
        {
            _index -= n;
            return *this;
        }

        friend Iterator operator-(const Iterator& it, const difference_type n)
        {
            Iterator tmp = it;
            tmp._index -= n;
            return tmp;
        }

        difference_type operator-(const Iterator& it)
        {
            return static_cast<difference_type>(_index - it._index);
        }

        reference operator[](const difference_type n) { return *(*this + n); }

        friend bool operator==(const Iterator& a, const Iterator& b)
        {
            return a._index == b._index;
        }

        friend auto operator<=>(const Iterator& a, const Iterator& b)
        {
            return a._index <=> b._index;
        }
    };

    void initializeWith(
        RE::TESDataHandler* dataHandler,
        const std::function<void(Transaction&)>& transaction);

    IteratorPair getSoulGemsWith(
        const SoulGemCapacity capacity,
        const SoulSize containedSoulSize) const
    {
        const auto& soulGemsAtCapacity = _soulGemMap.at(capacity);

        return {
            Iterator(soulGemsAtCapacity, containedSoulSize, 0),
            Iterator(
                soulGemsAtCapacity,
                containedSoulSize,
                soulGemsAtCapacity.size())};
    }

    void printContents() const;
};
