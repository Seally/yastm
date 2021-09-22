#include "EnchantItemFix.hpp"

#include <SKSE/SKSE.h>
#include <xbyak/xbyak.h>

/**
 * Check if memory has the expected bytes for patching.
 */
bool _isEnchantItemPatchable(std::uintptr_t baseAddress, std::uintptr_t offset)
{
    using namespace std::literals;
    namespace logger = SKSE::log;

    // Reuseable soul gem handling branch code.
    const std::uint8_t expected[] = {
        // clang-format off
        // .text:000000014086C862
        // loc_14086C862:
        0x48, 0x85, 0xc9,             // rcx is probably ExtraDataList
                                      // test    rcx, rcx            ; TEST performs an implied AND operation that does not modify the destination but sets CPU flags as if it did.
                                      //                             ; ANDing anything against itself produces itself, so this followed by JZ (Jump If Zero) is equivalent to the code:
                                      //                             ; if (rcx) { ...<jump destination>... }
        0x74, 0x05,                   // jz      short loc_14086C86C
        0x48, 0x8b, 0x09,             // mov     rcx, [rcx]          ; Dereferences rcx and assigns it to itself.
        0xeb, 0x03,                   // jmp     short loc_14086C86F
        // loc_14086C86C:
        0x49, 0x8b, 0xcd,             // mov     rcx, r13            ; r13 has been set to 0 for the scope of the function
        // loc_14086C86F:
        0x33, 0xd2,                   // xor     edx, edx            ; equivalent to mov edx, 0
        // clang-format on
    };

    if (std::memcmp(
            reinterpret_cast<std::uint8_t*>(
                static_cast<std::uintptr_t>(baseAddress + offset)),
            expected,
            sizeof expected) != 0) {
        logger::critical(
            "[ENCHANT] Expected bytes for reusable soul gem handling not found."sv);

        return false;
    }

    return true;
}

bool installEnchantItemFix()
{
    using namespace std::literals;
    namespace logger = SKSE::log;

    // CraftingSubMenus::EnchantMenu::EnchantItem
    // SkyrimSE.exe + 0x86c640 (v1.5.97)
    const REL::ID craftingSubMenus_enchantConstructMenu_enchantItem_id{50450};
    // SkyrimSE.exe + 0x2f26ef8 (v1.5.97)
    const REL::ID player_id{517014};

    constexpr std::uintptr_t patchOffset = 0x222;

    if (!_isEnchantItemPatchable(
            craftingSubMenus_enchantConstructMenu_enchantItem_id.address(),
            patchOffset)) {
        return false;
    }

    struct Patch : Xbyak::CodeGenerator {
        /**
         * @param[in] player_id                                             The REL::ID of the player.
         * @param[in] craftingSubMenus_enchantConstructMenu_enchantItem_id  The REL::ID of the function to patch.
         */
        explicit Patch(
            const REL::ID& player_id,
            const REL::ID& craftingSubMenus_enchantConstructMenu_enchantItem_id)
        {
            namespace logger = SKSE::log;
            constexpr std::uintptr_t stackSize = 0xb8;
            constexpr std::uintptr_t returnOffset = 0x236;

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
            mov(r10, player_id.address());
            mov(r10, ptr[r10]);

            // For details on how arguments are passed, see the x64 calling convention documentation
            // from Microsoft (specifically __fastcall).
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
            mov(rax, ptr[r10]);                    // rax <= player
            mov(ptr[rsp + stackSize - 0x98], r13); // a_fromRefr = 0
            mov(r9d, 1);                           // a_count = 1
            mov(r8, r13);                          // a_extraList = 0
            mov(rdx, ptr[rdi + 0x100]);            // a_object = soulGem.NAM0
            mov(rcx, r10);                         // this = player
            call(qword[rax + 0x2d0]); // PlayerCharacter::AddObjectToContainer

            // assign r10 to player (again, since r10 was not preserved in the last call).
            mov(r10, player_id.address());
            mov(r10, ptr[r10]);

            // re-fetch the original value for rcx since we just used it for the function call.
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
            //     char* ???,                      <= ???
            //     TESBoundObject * a_item,        <= soulGem
            //     std::int32_t a_count,           <= 1
            //     ITEM_REMOVE_REASON a_reason,    <= 0
            //     ExtraDataList * a_extraList,    <= soulGem's extraDataList (if it exists)
            //     TESObjectREFR * a_moveToRef,    <= 0
            //     const NiPoint3 * a_dropLoc = 0, <= 0
            //     const NiPoint3 * a_rotate = 0   <= 0
            // )
            mov(rax, ptr[r10]);                    // rax <= player
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
            dq(craftingSubMenus_enchantConstructMenu_enchantItem_id.address() +
               returnOffset);

            L(ifNAM0IsNullLabel);
            // Original branch code since we've overwritten some of it when performing the jump.
            test(rcx, rcx);
            jz(ifExtraDataListIsNullLabel2);
            mov(rcx, ptr[rcx]);
            jmp(ptr[rip + setSoulLabel]);

            L(ifExtraDataListIsNullLabel2);
            mov(rcx, r13);
            jmp(ptr[rip + setSoulLabel]);

            L(setSoulLabel);
            dq(craftingSubMenus_enchantConstructMenu_enchantItem_id.address() +
               0x22f);
        }
    };

    Patch patch{
        player_id,
        craftingSubMenus_enchantConstructMenu_enchantItem_id};
    patch.ready();

    logger::info(FMT_STRING("[ENCHANT] Patch size: {}"sv), patch.getSize());

    auto& trampoline = SKSE::GetTrampoline();
    // Code is significantly larger than the default trampoline size, so we need to allocate more.
    SKSE::AllocTrampoline(1 << 8);
    trampoline.write_branch<6>(
        craftingSubMenus_enchantConstructMenu_enchantItem_id.address() +
            patchOffset,
        trampoline.allocate(patch));

    return true;
}
