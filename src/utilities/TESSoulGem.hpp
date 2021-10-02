#ifndef UTILITIES_TESSOULGEM_HPP
#define UTILITIES_TESSOULGEM_HPP

#include <RE/T/TESSoulGem.h>
#include <RE/B/BGSDefaultObjectManager.h>
#include <RE/B/BGSKeyword.h>

inline RE::BGSKeyword* getReusableSoulGemKeyword()
{
    static const auto reusableSoulGemKeyword =
        RE::BGSDefaultObjectManager::GetSingleton()->GetObject<RE::BGSKeyword>(
            RE::DEFAULT_OBJECT::kKeywordReusableSoulGem);

    return reusableSoulGemKeyword;
}

inline bool canHoldBlackSoul(RE::TESSoulGem* const soulGemForm)
{
    return soulGemForm->GetFormFlags() &
           RE::TESSoulGem::RecordFlags::kCanHoldNPCSoul;
}

#endif // UTILITIES_TESSOULGEM_HPP
