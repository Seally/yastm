#ifndef YASTMCONFIG_HPP
#define YASTMCONFIG_HPP

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

private:
    std::vector<std::shared_ptr<SoulGemGroup>> _soulGemGroups;

    std::unordered_map<Key, float> _globalsDefaults;
    std::unordered_map<Key, GlobalId> _globals;

    std::array<std::vector<std::vector<RE::TESSoulGem*>>, SoulSize::Grand>
        _whiteSoulGems;
    std::vector<RE::TESSoulGem*> _blackSoulGemsEmpty;
    std::vector<RE::TESSoulGem*> _blackSoulGemsFilled;

    bool _soulGemMapCreated = false;

    explicit YASTMConfig()
    {
        _globalsDefaults[Key::AllowPartiallyFillingSoulGems] = 1;
        _globalsDefaults[Key::AllowSoulDisplacement] = 1;
        _globalsDefaults[Key::AllowSoulRelocation] = 1;
        _globalsDefaults[Key::AllowSoulShrinking] = 1;
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

    const std::vector<std::shared_ptr<SoulGemGroup>>& getSoulGemGroups() const
    {
        return _soulGemGroups;
    }

    const std::vector<RE::TESSoulGem*>&
        getSoulGemsWith(SoulSize capacity, SoulSize containedSoulSize) const;

    enum class Key {
        AllowPartiallyFillingSoulGems,
        AllowSoulDisplacement,
        AllowSoulRelocation,
        AllowSoulShrinking
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
        }

        return ""sv;
    }
};

#endif // YASTMCONFIG_HPP
