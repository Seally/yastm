#include "ChargeItemFix.hpp"

#include <xbyak/xbyak.h>

#include <SKSE/SKSE.h>
#include <REL/Relocation.h>
#include <RE/E/ExtraDataList.h>
#include <RE/P/PlayerCharacter.h>
#include <RE/T/TESSoulGem.h>

#include "global.hpp"
#include "expectedbytes.hpp"
#include "offsets.hpp"
#include "utilities/native.hpp"

namespace {
    /**
     * @brief Check if memory has the expected bytes for patching.
     */
    bool _isChargeItemPatchable()
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
} // namespace

bool installChargeItemFix()
{
    using namespace re::fix::chargeitem;
    using re::InventoryMenu::ChargeItem;

    if (!_isChargeItemPatchable()) {
        return false;
    }

    struct Patch : Xbyak::CodeGenerator {
        explicit Patch()
        {
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
            dq(reinterpret_cast<std::uint64_t>(_consumeReusableSoulGem));
        }
    };

    Patch patch;
    patch.ready();

    LOG_INFO_FMT("[CHARGE] Patch size: {}", patch.getSize());

    auto& trampoline = SKSE::GetTrampoline();
    SKSE::AllocTrampoline(1 << 7);
    trampoline.write_branch<6>(
        ChargeItem.address() + patchOffset,
        trampoline.allocate(patch));

    return true;
}
