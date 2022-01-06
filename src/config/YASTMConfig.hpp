#pragma once

#include <memory>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <RE/A/Actor.h>
#include <RE/B/BSCoreTypes.h>

#include <toml++/toml_table.h>

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
} // namespace RE

class YASTMConfig {
public:
    class Snapshot;
    using SoulGemGroupList = std::vector<SoulGemGroup>;
    template <typename KeyType>
    using GlobalVariableMap =
        std::unordered_map<KeyType, GlobalVariable<KeyType>>;

private:
    GlobalVariableMap<BoolConfigKey> _globalBools;
    GlobalVariableMap<EnumConfigKey> _globalEnums;

    SoulGemGroupList _soulGemGroupList;
    SoulGemMap _soulGemMap;

    std::unordered_map<DLLDependencyKey, const SKSE::PluginInfo*> _dependencies;
    mutable std::mutex mutex_;

    explicit YASTMConfig();

    /**
     * @brief Read and parse configuration files.
     */
    void loadConfigFiles_();
    /**
     * @brief Load game forms according to configuration. Call this *after*
     * loadConfigFiles_().
     */
    void loadGameForms_(RE::TESDataHandler* dataHandler);

    void _loadYASTMConfigFile();
    void _loadIndividualConfigFiles();
    std::size_t _readAndCountSoulGemGroupConfigs(const toml::table& table);

    void _loadGlobalForms(RE::TESDataHandler* dataHandler);
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
    void checkDllDependencies(const SKSE::LoadInterface* loadInterface);

    void loadConfig(RE::TESDataHandler* dataHandler);

    /**
     * @brief Clears (most) data stored in YASTMConfig. 
     */
    void clear();

    bool isDllLoaded(const DLLDependencyKey key) const
    {
        return _dependencies.contains(key) && _dependencies.at(key) != nullptr;
    }

    float getGlobalValue(const EnumConfigKey key) const
    {
        return _globalEnums.at(key).value();
    }
    float getGlobalValue(const BoolConfigKey key) const
    {
        return _globalBools.at(key).value();
    }

    bool getGlobalBool(const BoolConfigKey key) const
    {
        return getGlobalValue(key) != 0;
    }

    template <EnumConfigKey key>
    auto getGlobalEnum() const
    {
        return EnumConfigKeyTypeMap<key>()(getGlobalValue(key));
    }

    const SoulGemMap& soulGemMap() const { return _soulGemMap; }

    /**
     * @brief Represents a snapshot of the configuration at a certain point in
     * time.
     */
    class Snapshot {
        std::bitset<static_cast<std::size_t>(BoolConfigKey::Count)>
            _configBools;
        std::unordered_map<EnumConfigKey, EnumConfigUnderlyingType>
            _configEnums;

    public:
        explicit Snapshot(const YASTMConfig& config);

        template <EnumConfigKey K>
        auto get() const;

        bool operator[](BoolConfigKey key) const;
    };
};

inline YASTMConfig::Snapshot::Snapshot(const YASTMConfig& config)
{
    forEachBoolConfigKey([&, this](const BoolConfigKey key) {
        _configBools[static_cast<std::size_t>(key)] = config.getGlobalBool(key);
    });

    forEachEnumConfigKey([&, this](const EnumConfigKey key) {
        _configEnums.emplace(
            key,
            static_cast<EnumConfigUnderlyingType>(config.getGlobalValue(key)));
    });
}

inline bool YASTMConfig::Snapshot::operator[](const BoolConfigKey key) const
{
    return _configBools[static_cast<std::size_t>(key)];
}

template <EnumConfigKey K>
inline auto YASTMConfig::Snapshot::get() const
{
    return static_cast<EnumConfigKeyTypeMap<K>::type>(_configEnums.at(K));
}

class YASTMConfigLoadError : public std::runtime_error {
public:
    explicit YASTMConfigLoadError(const std::string& message)
        : std::runtime_error(message)
    {}
};
