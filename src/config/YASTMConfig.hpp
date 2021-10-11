#pragma once

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <RE/A/Actor.h>
#include <RE/B/BSCoreTypes.h>

#include <toml++/toml_table.h>

#include "ActorBase.hpp"
#include "ActorRef.hpp"
#include "ConfigKey.hpp"
#include "DllDependencyKey.hpp"
#include "GlobalVariable.hpp"
#include "SoulGemGroup.hpp"
#include "SoulSize.hpp"
#include "SoulGemMap.hpp"

namespace RE {
    class TESDataHandler;
    class TESGlobal;
    class TESSoulGem;
    class Actor;
    class TESNPC;
}

class YASTMConfig {
public:
    struct Snapshot;
    using SoulGemGroupList = std::vector<std::unique_ptr<SoulGemGroup>>;
    using ActorBaseList = std::vector<ActorBase>;
    using ActorRefList = std::vector<ActorRef>;

private:
    std::unordered_map<ConfigKey, GlobalVariable> _globals;

    ActorBaseList _actorBaseList;
    ActorRefList _actorRefList;
    std::unordered_set<RE::FormID> _diversionActorIgnoreList;

    SoulGemGroupList _soulGemGroupList;
    SoulGemMap _soulGemMap;

    std::unordered_map<DLLDependencyKey, const SKSE::PluginInfo*> _dependencies;

    explicit YASTMConfig();

    void _readYASTMConfig();
    void _readIndividualConfigs();
    void _readDiversionIgnoreConfigs(const toml::table& table);
    std::size_t _readAndCountSoulGemGroupConfigs(const toml::table& table);

    void _loadGlobalForms(RE::TESDataHandler* dataHandler);
    void _loadDiversionActorIgnoreList(RE::TESDataHandler* dataHandler);
    void _createSoulGemMap(RE::TESDataHandler* dataHandler);

public:
    YASTMConfig(const YASTMConfig&) = delete;
    YASTMConfig(YASTMConfig&&) = delete;
    YASTMConfig& operator=(const YASTMConfig&) = delete;
    YASTMConfig& operator=(YASTMConfig&&) = delete;

    static YASTMConfig& getInstance()
    {
        static YASTMConfig instance;

        return instance;
    }

    // These three functions needs to be called manually at different times.
    // loadGameForms() must be run only after readConfig finishes.
    void checkDllDependencies(const SKSE::LoadInterface* loadInterface);
    void readConfigs();
    void loadGameForms(RE::TESDataHandler* dataHandler);

    bool isDllLoaded(const DLLDependencyKey key) const
    {
        return _dependencies.contains(key) && _dependencies.at(key) != nullptr;
    }

    float getGlobalValue(const ConfigKey key) const
    {
        return _globals.at(key).value();
    }

    bool getGlobalBool(const ConfigKey key) const
    {
        return getGlobalValue(key) != 0;
    }

    const SoulGemGroupList& getSoulGemGroups() const
    {
        return _soulGemGroupList;
    }

    constexpr const std::vector<RE::TESSoulGem*>& getSoulGemsWith(
        const SoulSize capacity,
        const SoulSize containedSoulSize) const
    {
        return _soulGemMap.getSoulGemsWith(capacity, containedSoulSize);
    }

    bool isInDiversionIgnoreList(RE::Actor* const actor) const
    {
        return _diversionActorIgnoreList.contains(actor->GetFormID()) ||
               _diversionActorIgnoreList.contains(
                   actor->GetActorBase()->GetFormID());
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
        const bool allowSplitting;
        const bool allowExtraSoulRelocation;
        const bool allowSoulDiversion;
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
            getGlobalBool(ConfigKey::AllowSoulDiversion) &&
                getGlobalBool(ConfigKey::PerformSoulDiversionInDLL),
            getGlobalBool(ConfigKey::PreserveOwnership),
            getGlobalBool(ConfigKey::AllowNotifications)};
    }
};

class YASTMConfigLoadError : public std::runtime_error {
public:
    explicit YASTMConfigLoadError(const std::string& message);
};
