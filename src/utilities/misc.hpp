#pragma once

#include <RE/T/TESSoulGem.h>

namespace RE {
    class BGSKeyword;
}

RE::BGSKeyword* getReusableSoulGemKeyword();

inline bool canHoldBlackSoul(const RE::TESSoulGem* const soulGemForm)
{
    return soulGemForm->GetFormFlags() &
           RE::TESSoulGem::RecordFlags::kCanHoldNPCSoul;
}
