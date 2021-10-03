#ifndef CONFIG_YASTMCONFIG_HPP
#define CONFIG_YASTMCONFIG_HPP

#include <memory>
#include <unordered_map>
#include <vector>

#include "GlobalId.hpp"
#include "SoulGemGroup.hpp"
#include "SoulSize.hpp"

namespace RE {
    class TESDataHandler;
    class TESGlobal;
    class TESSoulGem;
}

class YASTMConfig {
public:
    enum class Key;
    struct Snapshot;
    typedef std::vector<std::unique_ptr<SoulGemGroup>> SoulGemGroupsList;

private:
    SoulGemGroupsList _soulGemGroups;

    std::unordered_map<Key, float> _globalsDefaults;
    std::unordered_map<Key, GlobalId> _globals;

    std::array<std::vector<std::vector<RE::TESSoulGem*>>, SoulSize::Grand>
        _whiteSoulGems;
    std::vector<RE::TESSoulGem*> _blackSoulGemsEmpty;
    std::vector<RE::TESSoulGem*> _blackSoulGemsFilled;

    bool _soulGemMapCreated = false;

    explicit YASTMConfig()
    {
        // Defaults used when no associated configuration key has been set up.
        _globalsDefaults[Key::AllowPartiallyFillingSoulGems] = 1;
        _globalsDefaults[Key::AllowSoulDisplacement] = 1;
        _globalsDefaults[Key::AllowSoulRelocation] = 1;
        _globalsDefaults[Key::AllowSoulShrinking] = 1;
        _globalsDefaults[Key::AllowExtraSoulRelocation] = 1;
        _globalsDefaults[Key::PreserveOwnership] = 1;
        _globalsDefaults[Key::AllowNotifications] = 1;
    }

    void _readYASTMConfig();
    void _readSoulGemConfigs();

    void _getGlobalForms(RE::TESDataHandler* dataHandler);
    void _createSoulGemMap(RE::TESDataHandler* dataHandler);
    bool _isValidConfig(RE::TESDataHandler* dataHandler) const;

public:
    static YASTMConfig& getInstance()
    {
        static YASTMConfig instance;

        return instance;
    }

    bool loadConfig();
    void processGameForms(RE::TESDataHandler* dataHandler);

    float getGlobalValue(const Key key) const;
    bool isPartialFillsAllowed() const;
    bool isSoulDisplacementAllowed() const;
    bool isSoulRelocationAllowed() const;
    bool isSoulShrinkingAllowed() const;
    bool isExtraSoulRelocationAllowed() const;
    bool preserveOwnership() const;
    bool isNotificationsAllowed() const;

    const SoulGemGroupsList& getSoulGemGroups() const { return _soulGemGroups; }

    const std::vector<RE::TESSoulGem*>&
        getSoulGemsWith(SoulSize capacity, SoulSize containedSoulSize) const;

    /**
     * @brief Represents a snapshot of the configuration at a certain point in
     * time.
     */
    struct Snapshot {
        const bool allowPartial;
        const bool allowDisplacement;
        const bool allowRelocation;
        const bool allowShrinking;
        const bool allowExtraSoulRelocation;
        const bool preserveOwnership;
        const bool allowNotifications;
    };

    Snapshot createSnapshot() const
    {
        return Snapshot{
            isPartialFillsAllowed(),
            isSoulDisplacementAllowed(),
            isSoulRelocationAllowed(),
            isSoulShrinkingAllowed(),
            isExtraSoulRelocationAllowed(),
            preserveOwnership(),
            isNotificationsAllowed()};
    }

    enum class Key {
        AllowPartiallyFillingSoulGems,
        AllowSoulDisplacement,
        AllowSoulRelocation,
        AllowSoulShrinking,
        AllowExtraSoulRelocation,
        PreserveOwnership,
        AllowNotifications,
    };

    static std::string_view toKeyName(const Key key)
    {
        using namespace std::literals;

        switch (key) {
        case Key::AllowPartiallyFillingSoulGems:
            return "allowPartiallyFillingSoulGems"sv;
        case Key::AllowSoulDisplacement:
            return "allowSoulDisplacement"sv;
        case Key::AllowSoulRelocation:
            return "allowSoulDisplacement"sv;
        case Key::AllowSoulShrinking:
            return "allowSoulShrinking"sv;
        case Key::AllowExtraSoulRelocation:
            return "allowExtraSoulRelocation"sv;
        case Key::PreserveOwnership:
            return "preserveOwnership"sv;
        case Key::AllowNotifications:
            return "allowNotifications"sv;
        }

        return ""sv;
    }
};

#endif // CONFIG_YASTMCONFIG_HPP
