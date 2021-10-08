#pragma once

#include <memory>
#include <unordered_map>
#include <vector>

#include "ConfigKey.hpp"
#include "GlobalVariable.hpp"
#include "SoulGemGroup.hpp"
#include "SoulSize.hpp"
#include "SoulGemMap.hpp"

namespace RE {
    class TESDataHandler;
    class TESGlobal;
    class TESSoulGem;
}

class YASTMConfig {
public:
    struct Snapshot;
    using SoulGemGroupsList = std::vector<std::unique_ptr<SoulGemGroup>>;

private:
    SoulGemGroupsList _soulGemGroups;
    std::unordered_map<ConfigKey, GlobalVariable> _globals;
    SoulGemMap _soulGemMap;

    explicit YASTMConfig();

    void _readYASTMConfig();
    void _readSoulGemConfigs();

    void _getGlobalForms(RE::TESDataHandler* dataHandler);
    void _createSoulGemMap(RE::TESDataHandler* dataHandler);

public:
    static YASTMConfig& getInstance()
    {
        static YASTMConfig instance;

        return instance;
    }

    void loadConfig();
    void processGameForms(RE::TESDataHandler* dataHandler);

    float getGlobalValue(const ConfigKey key) const
    {
        return _globals.at(key).value();
    }

    bool getGlobalBool(const ConfigKey key) const
    {
        return getGlobalValue(key) != 0;
    }

    const SoulGemGroupsList& getSoulGemGroups() const { return _soulGemGroups; }

    constexpr const std::vector<RE::TESSoulGem*>& getSoulGemsWith(
        const SoulSize capacity,
        const SoulSize containedSoulSize) const
    {
        return _soulGemMap.getSoulGemsWith(capacity, containedSoulSize);
    }

    /**
     * @brief Represents a snapshot of the configuration at a certain point in
     * time.
     */
    struct Snapshot {
        const bool allowPartial;
        const bool allowDisplacement;
        const bool allowRelocation;
        const bool allowShrinking;
        const bool allowSoulSplitting;
        const bool allowExtraSoulRelocation;
        const bool preserveOwnership;
        const bool allowNotifications;
    };

    Snapshot createSnapshot() const
    {
        return Snapshot{
            getGlobalBool(ConfigKey::AllowPartiallyFillingSoulGems),
            getGlobalBool(ConfigKey::AllowSoulDisplacement),
            getGlobalBool(ConfigKey::AllowSoulRelocation),
            getGlobalBool(ConfigKey::AllowSoulShrinking),
            getGlobalBool(ConfigKey::AllowSoulSplitting),
            getGlobalBool(ConfigKey::AllowExtraSoulRelocation),
            getGlobalBool(ConfigKey::PreserveOwnership),
            getGlobalBool(ConfigKey::AllowNotifications)};
    }
};

class YASTMConfigLoadError : public std::runtime_error {
public:
    explicit YASTMConfigLoadError(const std::string& message);
};
