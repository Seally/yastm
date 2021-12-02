#include "EnchantItemFix.hpp"

#include <SKSE/SKSE.h>
#include <xbyak/xbyak.h>

#include <RE/E/ExtraDataList.h>
#include <RE/P/PlayerCharacter.h>
#include <RE/T/TESSoulGem.h>

#include "global.hpp"
#include "expectedbytes.hpp"
#include "offsets.hpp"
#include "utilities/native.hpp"

namespace {
    /**
     * Check if memory has the expected bytes for patching.
     */
    bool _isEnchantItemPatchable()
    {
        using re::CraftingSubMenus::EnchantMenu::EnchantItem;
        using namespace re::fix::enchantitem;

        if (std::memcmp(
                reinterpret_cast<std::uint8_t*>(static_cast<std::uintptr_t>(
                    EnchantItem.address() + patchOffset)),
                expectedBytes,
                sizeof expectedBytes) != 0) {
            LOG_CRITICAL(
                "[ENCHANT] Expected bytes for reusable soul gem handling not "
                "found.");

            return false;
        }

        return true;
    }

    /**
     * @brief Creates a new ExtraDataList, copying some properties from the
     * original.
     */
    [[nodiscard]] RE::ExtraDataList* _createExtraDataListFromOriginal(
        RE::ExtraDataList* const originalExtraList)
    {
        if (originalExtraList != nullptr) {
            // Inherit ownership.
            if (const auto owner = originalExtraList->GetOwner(); owner) {
                const auto newExtraList = new RE::ExtraDataList();
                newExtraList->SetOwner(owner);
                return newExtraList;
            }
        }

        return nullptr;
    }

    void _consumeReusableSoulGem(
        RE::TESSoulGem* soulGemToConsume,
        RE::ExtraDataList** dataListPtr)
    {
        const auto dataList = dataListPtr ? *dataListPtr : nullptr;

        if ((dataList && dataList->GetSoulLevel() != RE::SOUL_LEVEL::kNone) ||
            soulGemToConsume->linkedSoulGem == nullptr) {
            native::BSExtraDataList::SetSoul(dataList, RE::SOUL_LEVEL::kNone);
            return;
        }

        const auto newDataList = _createExtraDataListFromOriginal(dataList);
        const auto player = RE::PlayerCharacter::GetSingleton();

        player->AddObjectToContainer(
            soulGemToConsume->linkedSoulGem,
            newDataList,
            1,
            nullptr);
        player->RemoveItem(
            soulGemToConsume,
            1,
            RE::ITEM_REMOVE_REASON::kRemove,
            dataList,
            nullptr);
    }
} // namespace

bool installEnchantItemFix()
{
    using re::CraftingSubMenus::EnchantMenu::EnchantItem;
    using namespace re::fix::enchantitem;

    if (!_isEnchantItemPatchable()) {
        return false;
    }

    struct Patch : Xbyak::CodeGenerator {
        explicit Patch()
        {
            // rcx = ExtraDataList**
            // rdi = TESSoulGem*
            // r13 = 0 (constant for this procedure)
            Xbyak::Label continueLabel;
            Xbyak::Label consumeReusableSoulGemLabel;

            // These labels duplicate the original branch's code.
            Xbyak::Label ifExtraDataListIsNullLabel2;
            Xbyak::Label setSoulLabel;

            // For details on how arguments are passed, see the x64 calling
            // convention documentation from Microsoft (especially for
            // __fastcall).
            //
            // - https://docs.microsoft.com/en-us/cpp/cpp/fastcall?view=msvc-160
            // - https://docs.microsoft.com/en-us/cpp/build/x64-software-conventions?view=msvc-160#register-usage
            mov(rdx, rcx); // BSExtraDataList**
            mov(rcx, rdi); // TESSoulGem*
            call(ptr[rip + consumeReusableSoulGemLabel]);

            jmp(ptr[rip + continueLabel]);

            L(continueLabel);
            dq(EnchantItem.address() + continueOffset);

            L(consumeReusableSoulGemLabel);
            dq(reinterpret_cast<std::uint64_t>(_consumeReusableSoulGem));
        }
    };

    Patch patch;
    patch.ready();

    LOG_INFO_FMT("[ENCHANT] Patch size: {}", patch.getSize());

    auto& trampoline = SKSE::GetTrampoline();
    SKSE::AllocTrampoline(1 << 7);
    trampoline.write_branch<6>(
        EnchantItem.address() + patchOffset,
        trampoline.allocate(patch));

    return true;
}
