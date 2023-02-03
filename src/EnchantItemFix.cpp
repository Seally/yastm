#include "enchantitemfix.hpp"

#include <SKSE/SKSE.h>
#include <xbyak/xbyak.h>

#include <RE/E/ExtraDataList.h>
#include <RE/M/Misc.h>
#include <RE/P/PlayerCharacter.h>
#include <RE/T/TESSoulGem.h>

#include "global.hpp"
#include "expectedbytes.hpp"
#include "messages.hpp"
#include "offsets.hpp"
#include "trampoline.hpp"
#include "config/configutilities.hpp"
#include "formatters/TESSoulGem.hpp"
#include "utilities/misc.hpp"
#include "utilities/native.hpp"

namespace {
    /**
     * Check if memory has the expected bytes for patching.
     */
    bool isEnchantItemPatchable_()
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

    void consumeReusableSoulGem_(
        RE::TESSoulGem* const soulGemToConsume,
        RE::ExtraDataList* const* const dataListPtr)
    {
        const auto dataList = dataListPtr ? *dataListPtr : nullptr;

        if (dataList && dataList->GetSoulLevel() != RE::SOUL_LEVEL::kNone) {
            native::BSExtraDataList::SetSoul(dataList, RE::SOUL_LEVEL::kNone);
            return;
        }

        const auto baseSoulGem = getSoulGemBaseForm(soulGemToConsume);

        // If we fail to get the base soul gem, we fall back to setting the
        // contained soul to zero on the extra data.
        if (baseSoulGem == nullptr) {
            if (dataList == nullptr) {
                // This should only happen on reusable soul gems that have no
                // NAM0 field specified, no entry in the soul gem map, and no
                // extra data.
                //
                // The last one is only possible if the reusable soul gem form
                // has a non-empty contained soul size, which isn't valid
                // without YASTM anyway, so reaching this is indication that
                // something has gone very wrong (in ESP/config files).
                RE::DebugNotification(
                    getMessage(MiscMessage::CannotFindSoulGemBaseForm));
                LOG_ERROR_FMT(
                    "[ENCHANT] Cannot find base form for soul gem {} and soul "
                    "gem has no extra data. Soul gem will not be consumed.",
                    *soulGemToConsume);
            } else {
                native::BSExtraDataList::SetSoul(
                    dataList,
                    RE::SOUL_LEVEL::kNone);
            }
            return;
        }

        auto newDataList = createExtraDataListFromOriginal(dataList);
        const auto player = RE::PlayerCharacter::GetSingleton();

        player->AddObjectToContainer(
            baseSoulGem,
            newDataList.release(), // Transfer ownership to the engine.
            1,
            nullptr);
        player->RemoveItem(
            soulGemToConsume,
            1,
            RE::ITEM_REMOVE_REASON::kRemove,
            dataList,
            nullptr);
    }

    struct Patch_ : Xbyak::CodeGenerator {
        explicit Patch_()
        {
            using re::CraftingSubMenus::EnchantMenu::EnchantItem;
            using namespace re::fix::enchantitem;

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
            dq(reinterpret_cast<std::uint64_t>(consumeReusableSoulGem_));
        }
    };
} // namespace

bool installEnchantItemFix()
{
    using re::CraftingSubMenus::EnchantMenu::EnchantItem;
    using namespace re::fix::enchantitem;

    if (!isEnchantItemPatchable_()) {
        return false;
    }

    Patch_ patch;
    patch.ready();

    LOG_INFO_FMT("[ENCHANT] Patch size: {}", patch.getSize());

    auto& trampoline = SKSE::GetTrampoline();
    allocateTrampoline();
    trampoline.write_branch<6>(
        EnchantItem.address() + patchOffset,
        trampoline.allocate(patch));

    return true;
}
