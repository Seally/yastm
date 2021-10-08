#pragma once

#include <array>
#include <vector>

#include "SoulSize.hpp"

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

    constexpr const std::vector<RE::TESSoulGem*>& getSoulGemsWith(
        const SoulSize capacity,
        const SoulSize containedSoulSize) const
    {
        using namespace std::literals;

#ifndef NDEBUG
        if (!isValidSoulCapacity(capacity) ||
            !isValidContainedSoulSize(capacity, containedSoulSize)) {
            throw InvalidSoulSpecificationError(capacity, containedSoulSize);
        }
#endif // NDEBUG

        if (capacity == SoulSize::Black) {
            if (containedSoulSize == SoulSize::None) {
                return _blackSoulGemsEmpty;
            } else if (containedSoulSize == SoulSize::Black) {
                return _blackSoulGemsFilled;
            }
        }

        return _whiteSoulGems[capacity - 1]
                             [static_cast<std::size_t>(containedSoulSize)];
    }

    void printContents() const;
};
