#ifndef UTILITIES_TESSOULGEM_HPP
#define UTILITIES_TESSOULGEM_HPP

namespace RE {
    class BGSKeyword;
    class TESSoulGem;
} // end namespace RE

RE::BGSKeyword* getReusableSoulGemKeyword();
bool canHoldBlackSoul(RE::TESSoulGem* const soulGemForm);

#endif // UTILITIES_TESSOULGEM_HPP
