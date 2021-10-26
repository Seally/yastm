#pragma once

namespace RE {
    class BGSKeyword;
    class TESSoulGem;
} // end namespace RE

RE::BGSKeyword* getReusableSoulGemKeyword();
bool canHoldBlackSoul(const RE::TESSoulGem* soulGemForm);
