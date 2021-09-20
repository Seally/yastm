#ifndef SOULGEMCONFIG_HPP
#define SOULGEMCONFIG_HPP

#include <filesystem>
#include <memory>
#include <optional>
#include <vector>

#include "SoulSize.hpp"
#include "SoulGemGroup.hpp"

namespace RE {
    class TESDataHandler;
    class TESSoulGem;
}

class SoulGemConfig {
    std::vector<std::shared_ptr<SoulGemGroup>> _soulGemGroups;

    std::array<std::vector<std::vector<RE::TESSoulGem*>>, SoulSize::Grand> _whiteSoulGems;
    std::vector<RE::TESSoulGem*> _blackSoulGemsEmpty;
    std::vector<RE::TESSoulGem*> _blackSoulGemsFilled;

    bool _soulGemMapCreated = false;

    explicit SoulGemConfig() {}

    void _readConfigFromFilesystem();
    void _createSoulGemMap(RE::TESDataHandler* dataHandler);
    bool _isValidConfig(RE::TESDataHandler* dataHandler) const;

public:
    static SoulGemConfig& getInstance() {
        static SoulGemConfig instance;

        return instance;
    }

    bool loadConfig();
    void createSoulGemMap(RE::TESDataHandler* dataHandler);

    const std::vector<std::shared_ptr<SoulGemGroup>>& getSoulGemGroups() const {
        return _soulGemGroups;
    }

    const std::vector<RE::TESSoulGem*>& getSoulGemsWith(SoulSize capacity, SoulSize containedSoulSize) const;
};

#endif // SOULGEMCONFIG_HPP
