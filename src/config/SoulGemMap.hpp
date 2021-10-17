#pragma once

#include <array>
#include <vector>

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
        static_cast<std::size_t>(SoulSize::Grand)>
        _whiteSoulGems;
    std::vector<RE::TESSoulGem*> _blackSoulGemsEmpty;
    std::vector<RE::TESSoulGem*> _blackSoulGemsFilled;

    bool _areListsInitialized = false;
    void initializeLists();

public:
    void addSoulGemGroup(
        const SoulGemGroup& group,
        RE::TESDataHandler* dataHandler);

    constexpr const std::vector<RE::TESSoulGem*>& getWhiteSoulGemsWith(
        const SoulSize capacity,
        const SoulSize containedSoulSize) const
    {
#ifndef NDEBUG
        if (!isValidSoulCapacity(capacity) ||
            !isValidContainedSoulSize(capacity, containedSoulSize)) {
            throw InvalidWhiteSoulSpecificationError(
                capacity,
                containedSoulSize);
        }
#endif // NDEBUG

        return _whiteSoulGems[capacity - 1]
                             [static_cast<std::size_t>(containedSoulSize)];
    }

    constexpr const std::vector<RE::TESSoulGem*>&
        getBlackSoulGemsWith(const SoulSize containedSoulSize) const
    {
        switch (containedSoulSize) {
        case SoulSize::None:
                return _blackSoulGemsEmpty;
        case SoulSize::Black:
                return _blackSoulGemsFilled;
            }

        throw InvalidBlackSoulSpecificationError(
            SoulSize::Black,
            containedSoulSize);
    }

    void printContents() const;
};
