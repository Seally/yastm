#pragma once

#include <array>
#include <compare>
#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>

#include "../global.hpp"
#include "SoulSize.hpp"
#include "ConcreteSoulGemGroup.hpp"
#include "SpecificationError.hpp"
#include "../utilities/EnumArray.hpp"

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
    using FormMap = EnumArray<
        SoulGemCapacity,
        ConcreteSoulGemGroupList>;
    FormMap soulGemMap_;

    friend class Iterator;

public:
    class Transaction {
        std::vector<std::reference_wrapper<const SoulGemGroup>> groupsToAdd_;

        friend class SoulGemMap;

        explicit Transaction() = default;
        Transaction(const Transaction&) = delete;
        Transaction(Transaction&&) = delete;
        Transaction& operator=(const Transaction&) = delete;
        Transaction& operator=(Transaction&&) = delete;

    public:
        void addSoulGemGroup(const SoulGemGroup& group)
        {
            groupsToAdd_.emplace_back(group);
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
        SoulSize containedSoulSize_;
        std::size_t index_;
        const ConcreteSoulGemGroupList* soulGemsAtCapacity_;

        explicit Iterator(
            const ConcreteSoulGemGroupList& soulGemsAtCapacity,
            const SoulSize containedSoulSize,
            const std::size_t index) noexcept
            : soulGemsAtCapacity_(&soulGemsAtCapacity)
            , containedSoulSize_(containedSoulSize)
            , index_(index)
        {}

        friend class SoulGemMap;

    public:
        Iterator(const Iterator& other) = default;
        Iterator(Iterator&& other) = default;
        Iterator& operator=(const Iterator& other) = default;
        Iterator& operator=(Iterator&& other) = default;

        const ConcreteSoulGemGroup& group() const
        {
            return *soulGemsAtCapacity_->at(index_);
        }

        const SoulSize containedSoulSize() const noexcept
        {
            return containedSoulSize_;
        }
        pointer get() const { return group().at(containedSoulSize_); }

        reference operator*() const { return *group().at(containedSoulSize_); }
        pointer operator->() const { return get(); }

        Iterator& operator++() noexcept
        {
            ++index_;
            return *this;
        }

        Iterator operator++(int) noexcept
        {
            Iterator tmp(*this);
            ++index_;
            return tmp;
        }

        Iterator& operator--() noexcept
        {
            --index_;
            return *this;
        }

        Iterator operator--(int) noexcept
        {
            Iterator tmp = *this;
            --index_;
            return tmp;
        }

        Iterator& operator+=(const difference_type n) noexcept
        {
            index_ += n;
            return *this;
        }

        friend Iterator
            operator+(const Iterator& it, const difference_type n) noexcept
        {
            Iterator tmp = it;
            tmp.index_ += n;
            return tmp;
        }

        Iterator& operator-=(const difference_type n) noexcept
        {
            index_ -= n;
            return *this;
        }

        friend Iterator
            operator-(const Iterator& it, const difference_type n) noexcept
        {
            Iterator tmp = it;
            tmp.index_ -= n;
            return tmp;
        }

        difference_type operator-(const Iterator& it) noexcept
        {
            return static_cast<difference_type>(index_ - it.index_);
        }

        reference operator[](const difference_type n) { return *(*this + n); }

        friend bool operator==(const Iterator& a, const Iterator& b) noexcept
        {
            return a.index_ == b.index_;
        }

        friend auto operator<=>(const Iterator& a, const Iterator& b) noexcept
        {
            return a.index_ <=> b.index_;
        }
    };

    void initializeWith(
        RE::TESDataHandler* dataHandler,
        const std::function<void(Transaction&)>& transaction);

    void clear();

    IteratorPair getSoulGemsWith(
        const SoulGemCapacity capacity,
        const SoulSize containedSoulSize) const
    {
        const auto& soulGemsAtCapacity = soulGemMap_.at(capacity);

        return {
            Iterator(soulGemsAtCapacity, containedSoulSize, 0),
            Iterator(
                soulGemsAtCapacity,
                containedSoulSize,
                soulGemsAtCapacity.size())};
    }

    void printContents() const;
};
