#include "TESSoulGem.hpp"

#include <RE/T/TESSoulGem.h>
#include <RE/B/BGSDefaultObjectManager.h>
#include <RE/B/BGSKeyword.h>

RE::BGSKeyword* getReusableSoulGemKeyword()
{
    static const auto reusableSoulGemKeyword =
        RE::BGSDefaultObjectManager::GetSingleton()->GetObject<RE::BGSKeyword>(
            RE::DEFAULT_OBJECT::kKeywordReusableSoulGem);

    return reusableSoulGemKeyword;
}

bool canHoldBlackSoul(const RE::TESSoulGem* const soulGemForm)
{
    return soulGemForm->GetFormFlags() &
           RE::TESSoulGem::RecordFlags::kCanHoldNPCSoul;
}
