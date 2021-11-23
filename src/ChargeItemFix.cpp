#include "ChargeItemFix.hpp"

#include <SKSE/SKSE.h>
#include <xbyak/xbyak.h>

#include "global.hpp"
#include "expectedbytes.hpp"
#include "offsets.hpp"

/**
 * @brief Check if memory has the expected bytes for patching.
 */
bool _isChargeItemPatchable()
{
    using namespace re::fix::chargeitem;
    using re::InventoryMenu::ChargeItem;

    if (std::memcmp(
            reinterpret_cast<std::uint8_t*>(static_cast<std::uintptr_t>(
                ChargeItem.address() + beginOffset)),
            expectedBytes,
            sizeof expectedBytes) != 0) {
        LOG_CRITICAL(
            "[CHARGE] Expected bytes for reusable soul gem handling not "
            "found.");

        return false;
    }

    return true;
}

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
            // Pseudocode:
            // if (soulGem->NAM0 == null) {
            //     <go to original code>
            // } else {
            //     player->AddObjectToContainer(
            //         item      = soulGem->NAM0,
            //         extraList = null,
            //         count     = 1,
            //         fromRefr  = null
            //     );
            //     updateInventory(player, soulGem->NAM0)
            //     player->RemoveItem(
            //         ???,
            //         item      = soulGem,
            //         count     = 1,
            //         reason    = 0,
            //         extraList = soulGemExtraDataList,
            //         moveToRef = null,
            //         dropLoc   = null,
            //         rotate    = null
            //     );
            // }

            // rax = ExtraDataList* (probably)
            // rbx = TESSoulGem*
            // r12 = 0 (constant for this procedure)
            Xbyak::Label ifExtraDataListIsNullLabel;
            Xbyak::Label ifNAM0IsNullLabel;
            Xbyak::Label removeItemLabel;
            Xbyak::Label returnContinueLabel;

            // These labels duplicate the original branch's code.
            Xbyak::Label ifExtraDataListIsNullLabel2;
            Xbyak::Label setSoulLabel;

            // These labels reference external functions.
            Xbyak::Label updateInventoryLabel;

            // Check the NAM0 entry for the soul gem.
            // clang-format off
            mov(rcx, ptr[rbx + 0x100]); // rbx = soulGem, [rbx + 100h] = soulGem.NAM0
            test(rcx, rcx);             // ZF = 1 if rax is 0.
            // clang-format on
            jz(ifNAM0IsNullLabel, T_NEAR);

            // assign r10 to player
            mov(r10, player.address());
            mov(r10, ptr[r10]);

            // For details on how arguments are passed, see the x64 calling
            // convention documentation from Microsoft (especially for
            // __fastcall).
            //
            // - https://docs.microsoft.com/en-us/cpp/cpp/fastcall?view=msvc-160
            // - https://docs.microsoft.com/en-us/cpp/build/x64-software-conventions?view=msvc-160#register-usage

            // Case: TESSoulGem has NAM0 defined.
            //
            // PlayerCharacter::AddObjectToContainer(
            //     TESBoundObject * a_object,
            //     ExtraDataList * a_extraList,
            //     std::int32_t a_count,
            //     TESObjectREFR * a_fromRefr
            // )
            mov(rax, ptr[r10]); // rax <- player
            mov(ptr[rsp + stackSize - 0xa8], r12); // a_fromRefr = 0
            mov(r9d, 1); // a_count = 1
            mov(r8, r12); // a_extraList = 0
            mov(rdx, ptr[rbx + 0x100]); // a_object = soulGem.NAM0
            mov(rcx, r10); // this = player
            call(qword[rax + 0x2d0]); // PlayerCharacter::AddObjectToContainer

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
            mov(rdx, ptr[rbx + 0x100]);
            mov(rcx, player.address());
            mov(rcx, ptr[rcx]);
            call(ptr[rip + updateInventoryLabel]);

            // assign r10 to player (again, since r10 may not be preserved).
            mov(r10, player.address());
            mov(r10, ptr[r10]);

            // re-fetch the original value for rax since we overwrote it earlier.
            mov(rax, ptr[r15 + 0x8]); // rax is probably extraDataList

            test(rax, rax);
            jz(ifExtraDataListIsNullLabel);
            mov(rcx, ptr[rax]); // dereference
            jmp(removeItemLabel);

            L(ifExtraDataListIsNullLabel);
            mov(rcx, r12);

            L(removeItemLabel);
            // PlayerCharacter::RemoveItem(
            //     char* ???,                      <- ???
            //     TESBoundObject * a_item,        <- soulGem
            //     std::int32_t a_count,           <- 1
            //     ITEM_REMOVE_REASON a_reason,    <- 0
            //     ExtraDataList * a_extraList,    <- soulGem's extraDataList (if it exists)
            //     TESObjectREFR * a_moveToRef,    <- 0
            //     const NiPoint3 * a_dropLoc = 0, <- 0
            //     const NiPoint3 * a_rotate = 0   <- 0
            // )
            // clang-format off
            mov(rax, ptr[r10]);                       // rax <- player
            mov(ptr[rsp + stackSize - 0x88], r12);    // a_rotate = 0
            mov(ptr[rsp + stackSize - 0x90], r12);    // a_dropLoc = 0
            mov(ptr[rsp + stackSize - 0x98], r12);    // a_moveToRef = 0
            mov(ptr[rsp + stackSize - 0xa0], rcx);    // a_extraList = soulGem's extraDataList (if it exists)
            mov(dword[rsp + stackSize - 0xa8], r12d); // a_reason = 0
            mov(r9d, 1);                              // a_count = 1
            mov(r8, rbx);                             // a_item = soulGem
            lea(rdx, ptr[rsp + stackSize + 0x8]);     // ???
            mov(rcx, r10);                            // this = player
            call(qword[rax + 0x2b0]);                 // PlayerCharacter::RemoveItem
            // clang-format on
            jmp(ptr[rip + returnContinueLabel]);

            L(returnContinueLabel);
            dq(ChargeItem.address() + successContinueOffset);

            L(ifNAM0IsNullLabel);
            // Original branch code since we've overwritten some of it when
            // performing the jump.
            test(rax, rax);
            jz(ifExtraDataListIsNullLabel2);
            mov(rcx, ptr[rax]);
            jmp(ptr[rip + setSoulLabel]);

            L(ifExtraDataListIsNullLabel2);
            mov(rcx, r12);
            jmp(ptr[rip + setSoulLabel]);

            L(setSoulLabel);
            dq(ChargeItem.address() + noLinkedSoulGemContinueOffset);

            jmp(ptr[rip + returnContinueLabel]);

            L(updateInventoryLabel);
            dq(updateInventory.address());
        }
    };

    Patch patch;
    patch.ready();

    LOG_INFO_FMT("[CHARGE] Patch size: {}", patch.getSize());

    auto& trampoline = SKSE::GetTrampoline();
    // Patch code is significantly larger than the default trampoline size,
    // so we need to allocate more.
    SKSE::AllocTrampoline(1 << 8);
    trampoline.write_branch<6>(
        ChargeItem.address() + beginOffset,
        trampoline.allocate(patch));

    return true;
}
