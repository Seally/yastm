#include <xbyak/xbyak.h>
#include "EnchantItemFix.h"
#include "offsets.h"

namespace YASTM {
	/**
	 * Check if memory has the expected bytes for patching.
	 */
	bool isEnchantItemPatchable(std::uintptr_t baseAddress) {
		namespace logger = SKSE::log;

		constexpr std::uintptr_t offset = 0x222;

		// Reuseable soul gem handling branch code.
		const std::uint8_t expected[] = {
			// .text:000000014086C862
			// loc_14086C862:
			0x48, 0x85, 0xc9,             // test    rcx, rcx            ; TEST performs an implied AND operation that does not modify the destination but sets CPU flags as if it did.
										  //                             ; ANDing anything against itself produces itself, so this followed by JZ (Jump If Zero) is equivalent to the code:
										  //                             ; if (rcx) { ...<jump destination>... }
			0x74, 0x05,                   // jz      short loc_14086C86C
			0x48, 0x8b, 0x09,             // mov     rcx, [rcx]          ; Dereferences rcx and assigns it to itself.
			0xeb, 0x03,                   // jmp     short loc_14086C86F
			// loc_14086C86C:
			0x49, 0x8b, 0xcd,             // mov     rcx, r13            ; r13 has been set to 0 for the scope of the function
			// loc_14086C86F:
			0x33, 0xd2,                   // xor     edx, edx            ; equivalent to mov edx, 0

			// Commenting this test out because E8 is a near, relative call, which can change when the new codebase
			// is changed.
			//0xe8, 0xea, 0x20, 0x8a, 0xff, // call    sub_14010E960       ; BSExtraDataList::SetSoul(rcx, edx)
		};

		if (std::memcmp(reinterpret_cast<std::uint8_t*>(static_cast<std::uintptr_t>(baseAddress + offset)), expected, sizeof expected) != 0) {
			logger::critical("[ENCHANT] Expected bytes for reusable soul gem handling not found.");

			return false;
		}

		return true;
	}

	bool InstallEnchantItemFix() {
		namespace logger = SKSE::log;

		// CraftingSubMenus::EnchantMenu::EnchantItem
		const REL::ID craftingSubMenus_enchantConstructMenu_enchantItem_id{50450}; // SkyrimSE.exe + 0x86c640 (v1.5.97)
		const REL::ID player_id{517014};                                           // SkyrimSE.exe + 0x2f26ef8 (v1.5.97)

		if (!isEnchantItemPatchable(craftingSubMenus_enchantConstructMenu_enchantItem_id.address())) {
			return false;
		}

		struct Patch : Xbyak::CodeGenerator {
			/**
			 * @param[in] baseAddress          The starting address of the procedure to patch.
			 */
			Patch(const REL::ID& player_id, const REL::ID& craftingSubMenus_enchantConstructMenu_enchantItem_id) {
				namespace logger = SKSE::log;
				constexpr std::uintptr_t stackSize = 0xb8;
				constexpr std::uintptr_t originalBranchOffset = 0x222;
				constexpr std::uintptr_t returnOffset = 0x236;

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
				mov(rax, ptr[rdi + 0x100]); // rdi = soulGem, [rdi + 100h] = soulGem.NAM0
				test(rax, rax);             // ZF = 1 if rax is 0.
				jz(ifNAM0IsNullLabel, T_NEAR);

				// assign r10 to player
				mov(r10, player_id.address());
				mov(r10, ptr[r10]);

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
				call(qword[rax + 0x2d0]);              // PlayerCharacter::AddObjectToContainer

				// assign r10 to player (again, since r10 was not preserved in the last call).
				mov(r10, player_id.address());
				mov(r10, ptr[r10]);

				// re-fetch the original value for rcx since we just used it for the function call.
				mov(rax, ptr[rbx + 0x18]);
				mov(rcx, ptr[rax + 0x8]);  // rcx is probably extraDataList

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
				//     ExtraDataList * a_extraList,    <= 0
				//     TESObjectREFR * a_moveToRef,    <= 0
				//     const NiPoint3 * a_dropLoc = 0, <= 0
				//     const NiPoint3 * a_rotate = 0   <= 0
				// )
				mov(rax, ptr[r10]);                       // rax <= player
				mov(ptr[rsp + stackSize - 0x78], r13);    // a_rotate = 0
				mov(ptr[rsp + stackSize - 0x80], r13);    // a_dropLoc = 0
				mov(ptr[rsp + stackSize - 0x88], r13);    // a_moveToRef = 0
				mov(ptr[rsp + stackSize - 0x90], rdx);    // a_extraList = 0
				mov(dword[rsp + stackSize - 0x98], r13d); // a_reason = 0
				mov(r9d, 1);                              // a_count = 1
				mov(r8, rdi);                             // a_item = soulGem
				lea(rdx, ptr[rsp + stackSize + 0x8]);     // ???
				mov(rcx, r10);                            // this = player
				call(qword[rax + 0x2b0]);                 // PlayerCharacter::RemoveItem
				jmp(ptr[rip + returnContinueLabel]);

				L(returnContinueLabel);
				dq(craftingSubMenus_enchantConstructMenu_enchantItem_id.address() + returnOffset);

				L(ifNAM0IsNullLabel);
				// Original branch code
				test(rcx, rcx);
				jz(ifExtraDataListIsNullLabel2);
				mov(rcx, ptr[rcx]);
				jmp(ptr[rip + setSoulLabel]);

				L(ifExtraDataListIsNullLabel2);
				mov(rcx, r13);
				jmp(ptr[rip + setSoulLabel]);

				L(setSoulLabel);
				dq(craftingSubMenus_enchantConstructMenu_enchantItem_id.address() + 0x22f);
			}
		};

		Patch patch{player_id, craftingSubMenus_enchantConstructMenu_enchantItem_id};
		patch.ready();

		logger::info("Patch size: {}", patch.getSize());

		auto& trampoline = SKSE::GetTrampoline();
		// Code is significantly larger than the default trampoline size, so we need to allocate more.
		SKSE::AllocTrampoline(1 << 8);
		trampoline.write_branch<6>(
			craftingSubMenus_enchantConstructMenu_enchantItem_id.address() + 0x222,
			trampoline.allocate(patch)
		);

		return true;
	}

	//bool InstallChargeFix() {
	//	const auto address = Offsets::CraftingSubMenus::EnchantConstructMenu::EnchantItem.address();
	//	const std::uintptr_t CODE_OFFSET = 0x2a5;
	//	const std::uint8_t expected[] = {
	//		// loc_14088EB35:
	//		0x48, 0x85, 0xc9,             // test    rax, rax
	//		0x74, 0x05,                   // jz      short loc_14088EB3F
	//		0x48, 0x8b, 0x08,             // mov     rcx, [rax]
	//		0xeb, 0x03,                   // jmp     short loc_14088EB42
	//		// loc_14088EB3F:
	//		0x49, 0x8b, 0xcc,             // mov     rcx, r12
	//		// loc_14088EB42:
	//		0x33, 0xd2,                   // xor     edx, edx      ; equivalent to mov edx, 0
	//		0xe8, 0x17, 0xfe, 0x87, 0xff, // call    sub_14010E960 ; BSExtraDataList::SetSoul(rcx, edx)
	//	};

	//	if (std::memcmp(reinterpret_cast<std::uint8_t*>(static_cast<std::uintptr_t>(address + CODE_OFFSET)), expected, sizeof expected) != 0) {
	//		// Expected bytes not found.

	//		return false;
	//	}

	//	auto& trampoline = SKSE::GetTrampoline();

	//	return true;
	//}
} // namespace YASTM
