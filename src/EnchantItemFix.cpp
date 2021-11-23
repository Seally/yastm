#include "EnchantItemFix.hpp"

#include <SKSE/SKSE.h>
#include <xbyak/xbyak.h>

#include "global.hpp"
#include "expectedbytes.hpp"
#include "offsets.hpp"

/**
 * Check if memory has the expected bytes for patching.
 */
bool _isEnchantItemPatchable()
{
    using re::CraftingSubMenus::EnchantMenu::EnchantItem;
    using namespace re::fix::enchantitem;

    if (std::memcmp(
            reinterpret_cast<std::uint8_t*>(static_cast<std::uintptr_t>(
                EnchantItem.address() + beginOffset)),
            expectedBytes,
            sizeof expectedBytes) != 0) {
        LOG_CRITICAL(
            "[ENCHANT] Expected bytes for reusable soul gem handling not "
            "found.");

        return false;
    }

    return true;
}

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

            // rcx = ExtraDataList* (probably)
            // rdi = TESSoulGem*
            // r13 = 0 (constant for this procedure)
            Xbyak::Label ifExtraDataListIsNullLabel;
            Xbyak::Label ifNAM0IsNullLabel;
            Xbyak::Label removeItemLabel;
            Xbyak::Label returnContinueLabel;

            // These labels duplicate the original branch's code.
            Xbyak::Label ifExtraDataListIsNullLabel2;
            Xbyak::Label setSoulLabel;

            // Check the NAM0 entry for the soul gem.
            mov(rax,
                ptr[rdi + 0x100]); // rdi = soulGem, [rdi + 100h] = soulGem.NAM0
            test(rax, rax);        // ZF = 1 if rax is 0.
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

            // TESSoulGem has NAM0 defined.
            // PlayerCharacter::AddObjectToContainer(
            //     TESBoundObject * a_object,
            //     ExtraDataList * a_extraList,
            //     std::int32_t a_count,
            //     TESObjectREFR * a_fromRefr
            // )
            mov(rax, ptr[r10]);                    // rax <- player
            mov(ptr[rsp + stackSize - 0x98], r13); // a_fromRefr = 0
            mov(r9d, 1);                           // a_count = 1
            mov(r8, r13);                          // a_extraList = 0
            mov(rdx, ptr[rdi + 0x100]);            // a_object = soulGem.NAM0
            mov(rcx, r10);                         // this = player
            call(qword[rax + 0x2d0]); // PlayerCharacter::AddObjectToContainer

            // assign r10 to player (again, since r10 was not preserved in the
            // last call).
            mov(r10, player.address());
            mov(r10, ptr[r10]);

            // re-fetch the original value for rcx since we just used it for the
            // function call.
            mov(rax, ptr[rbx + 0x18]);
            mov(rcx, ptr[rax + 0x8]); // rcx is probably extraDataList

            test(rcx, rcx);
            jz(ifExtraDataListIsNullLabel);
            mov(rdx, ptr[rcx]); // dereference
            jmp(removeItemLabel);

            L(ifExtraDataListIsNullLabel);
            mov(rdx, r13);

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
            mov(rax, ptr[r10]);                    // rax <- player
            mov(ptr[rsp + stackSize - 0x78], r13); // a_rotate = 0
            mov(ptr[rsp + stackSize - 0x80], r13); // a_dropLoc = 0
            mov(ptr[rsp + stackSize - 0x88], r13); // a_moveToRef = 0
            mov(ptr[rsp + stackSize - 0x90],
                rdx); // a_extraList = soulGem's extraDataList (if it exists)
            mov(dword[rsp + stackSize - 0x98], r13d); // a_reason = 0
            mov(r9d, 1);                              // a_count = 1
            mov(r8, rdi);                             // a_item = soulGem
            lea(rdx, ptr[rsp + stackSize + 0x8]);     // ???
            mov(rcx, r10);                            // this = player
            call(qword[rax + 0x2b0]); // PlayerCharacter::RemoveItem
            jmp(ptr[rip + returnContinueLabel]);

            L(returnContinueLabel);
            dq(EnchantItem.address() + successContinueOffset);

            L(ifNAM0IsNullLabel);
            // Original branch code since we've overwritten some of it when
            // performing the jump.
            test(rcx, rcx);
            jz(ifExtraDataListIsNullLabel2);
            mov(rcx, ptr[rcx]);
            jmp(ptr[rip + setSoulLabel]);

            L(ifExtraDataListIsNullLabel2);
            mov(rcx, r13);
            jmp(ptr[rip + setSoulLabel]);

            L(setSoulLabel);
            dq(EnchantItem.address() + noLinkedSoulGemContinueOffset);
        }
    };

    Patch patch;
    patch.ready();

    LOG_INFO_FMT("[ENCHANT] Patch size: {}", patch.getSize());

    auto& trampoline = SKSE::GetTrampoline();
    // Code is significantly larger than the default trampoline size, so we need
    // to allocate more.
    SKSE::AllocTrampoline(1 << 8);
    trampoline.write_branch<6>(
        EnchantItem.address() + beginOffset,
        trampoline.allocate(patch));

    return true;
}
