#include "ChargeItemFix.hpp"

#include <xbyak/xbyak.h>

#include <SKSE/SKSE.h>
#include <REL/Relocation.h>
#include <RE/E/ExtraDataList.h>
#include <RE/M/Misc.h>
#include <RE/P/PlayerCharacter.h>
#include <RE/T/TESSoulGem.h>

#include "global.hpp"
#include "expectedbytes.hpp"
#include "offsets.hpp"
#include "trampoline.hpp"
#include "config/utilities.hpp"
#include "formatters/TESSoulGem.hpp"
#include "trapsoul/messages.hpp"
#include "utilities/misc.hpp"
#include "utilities/native.hpp"

namespace {
    /**
     * @brief Check if memory has the expected bytes for patching.
     */
    bool isChargeItemPatchable_()
    {
        using namespace re::fix::chargeitem;
        using re::InventoryMenu::ChargeItem;

        if (std::memcmp(
                reinterpret_cast<std::uint8_t*>(static_cast<std::uintptr_t>(
                    ChargeItem.address() + patchOffset)),
                expectedBytes,
                sizeof expectedBytes) != 0) {
            LOG_CRITICAL(
                "[CHARGE] Expected bytes for reusable soul gem handling not "
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

        // This soul gem uses extra data to store the contained soul size,
        // so we set that instead.
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
                    "[CHARGE] Cannot find base form for soul gem {} and soul "
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

        // Updates the inventory UI. If we don't call this, the added soul
        // gem won't show up until the user reopens the inventory menu.
        //
        // It seems we need to call this before removing the item, otherwise
        // this will do nothing.
        //
        // 1st argument of the function seems to be the actor in question,
        // and the 2nd the item to add. When removing items, the 2nd
        // argument should be NULL.
        //
        // This function is called for the item remove case already, but not
        // for the added item (since it doesn't originally call it at all).
        //
        // Note: In the disassembly, the "remove item" version of this
        //       function is called within the function at:
        //
        //           SkyrimSE.exe + 856a50 [1.5.97.0]  [ADDRLIB:50099]
        //           SkyrimSE.exe + 883930 [1.6.318.0] [ADDRLIB:51031]
        //           SkyrimSE.exe + 8836a0 [1.6.323.0] [ADDRLIB:51031]
        native::updateInventory(player, soulGemToConsume->linkedSoulGem);

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
            using namespace re::fix::chargeitem;
            using re::InventoryMenu::ChargeItem;

            // rax = BSExtraDataList**
            // rbx = TESSoulGem*
            // r12 = 0 (constant for this procedure)
            Xbyak::Label continueLabel;
            Xbyak::Label consumeReusableSoulGemLabel;

            // For details on how arguments are passed, see the x64 calling
            // convention documentation from Microsoft (especially for
            // __fastcall).
            //
            // - https://docs.microsoft.com/en-us/cpp/cpp/fastcall?view=msvc-160
            // - https://docs.microsoft.com/en-us/cpp/build/x64-software-conventions?view=msvc-160#register-usage
            mov(rdx, rax); // BSExtraDataList**
            mov(rcx, rbx); // TESSoulGem*
            call(ptr[rip + consumeReusableSoulGemLabel]);

            jmp(ptr[rip + continueLabel]);

            L(continueLabel);
            dq(ChargeItem.address() + continueOffset);

            L(consumeReusableSoulGemLabel);
            dq(reinterpret_cast<std::uint64_t>(consumeReusableSoulGem_));
        }
    };
} // namespace

bool installChargeItemFix()
{
    using namespace re::fix::chargeitem;
    using re::InventoryMenu::ChargeItem;

    if (!isChargeItemPatchable_()) {
        return false;
    }

    Patch_ patch;
    patch.ready();

    LOG_INFO_FMT("[CHARGE] Patch size: {}", patch.getSize());
    allocateTrampoline();
    auto& trampoline = SKSE::GetTrampoline();
    trampoline.write_branch<6>(
        ChargeItem.address() + patchOffset,
        trampoline.allocate(patch));

    return true;
}
