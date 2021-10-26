#pragma once

#include <array>
#include <functional>
#include <vector>

#include "../global.hpp"
#include "SoulSize.hpp"
#include "SpecificationError.hpp"

namespace RE {
    class TESDataHandler;
    class TESSoulGem;
} // end namespace RE

class SoulGemGroup;

class SoulGemMap {
    std::array<
        std::vector<std::vector<RE::TESSoulGem*>>,
        static_cast<std::size_t>(SoulSize::Black)>
        _whiteSoulGems;
    std::vector<RE::TESSoulGem*> _pureBlackSoulGemsEmpty;
    std::vector<RE::TESSoulGem*> _pureBlackSoulGemsFilled;

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

    void initializeWith(
        RE::TESDataHandler* dataHandler,
        const std::function<void(Transaction&)>& transaction);

    const std::vector<RE::TESSoulGem*>& getWhiteSoulGemsWith(
        const SoulSize capacity,
        const SoulSize containedSoulSize) const
    {
        try {
            return _whiteSoulGems.at(capacity - 1)
                .at(static_cast<std::size_t>(containedSoulSize));
        } catch (...) {
            std::throw_with_nested(InvalidWhiteSoulSpecificationError(
                capacity,
                containedSoulSize));
        }
    }

    constexpr const std::vector<RE::TESSoulGem*>&
        getPureBlackSoulGemsWith(const SoulSize containedSoulSize) const
    {
        if (containedSoulSize == SoulSize::None) {
            return _pureBlackSoulGemsEmpty;
        }

        if (containedSoulSize == SoulSize::Black) {
            return _pureBlackSoulGemsFilled;
        }

        throw InvalidBlackSoulSpecificationError(
            SoulSize::Black,
            containedSoulSize);
    }

    void printContents() const;
};
