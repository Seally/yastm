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
    void addSoulGemGroup(const SoulGemGroup& group, RE::TESDataHandler* dataHandler);

    const std::vector<RE::TESSoulGem*>&
        getSoulGemsWith(SoulSize capacity, SoulSize containedSoulSize) const;

    void printContents() const;
};
