#include "soultraputils.hpp"

RE::BGSKeyword* getReusableSoulGemKeyword()
{
    // I don't know why putting this in a .cpp file stops Visual Studio from
    // thinking GetObject is a macro from wingdi.h.
    static const auto reusableSoulGemKeyword =
        RE::BGSDefaultObjectManager::GetSingleton()->GetObject<RE::BGSKeyword>(
            RE::DEFAULT_OBJECT::kKeywordReusableSoulGem);

    return reusableSoulGemKeyword;
}
