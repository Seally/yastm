#pragma once

#include <REL/Relocation.h>

#include "global.hpp"

// Note: The listed offsets in the comments only contain addresses for binaries
//       I've personally verified. Rely on the ADDRLIB ID to find addresses for
//       other binaries.

namespace re {
    namespace SoulsTrapped {
        // SkyrimSE.exe + 0x636c40 [1.5.97.0]  [ADDRLIB:37916]
        // SkyrimSE.exe + 0x65c9e0 [1.6.318.0] [ADDRLIB:38873]
        // SkyrimSE.exe + 0x65c750 [1.6.323.0] [ADDRLIB:38873]
        constexpr REL::ID GetEventSource(VERSION_SPECIFIC(37916, 38873));
    } // namespace SoulsTrapped

    namespace BSExtraDataList {
        constexpr REL::ID SetSoul(VERSION_SPECIFIC(11474, 11620));
    }

    namespace CraftingSubMenus {
        namespace EnchantMenu {
            // CraftingSubMenus::EnchantMenu::EnchantItem
            //
            // SkyrimSE.exe + 0x86c640 [1.5.97.0]  [ADDRLIB:50450]
            // SkyrimSE.exe + 0x89a9c0 [1.6.318.0] [ADDRLIB:51355]
            // SkyrimSE.exe + 0x89a730 [1.6.323.0] [ADDRLIB:51355]
            constexpr REL::ID EnchantItem(VERSION_SPECIFIC(50450, 51355));
        } // namespace EnchantMenu
    } // namespace CraftingSubMenus

    namespace InventoryMenu {
        // SkyrimSE.exe + 0x88e890 [1.5.97.0]  [ADDRLIB:50980]
        // SkyrimSE.exe + 0x8bdfe0 [1.6.318.0] [ADDRLIB:51859]
        // SkyrimSE.exe + 0x8bdd50 [1.6.323.0] [ADDRLIB:51859]
        constexpr REL::ID ChargeItem(VERSION_SPECIFIC(50980, 51859));
    } // namespace InventoryMenu

    namespace soultraputils {
        namespace Actor {
            // Possibly related to/same as Actor::CalculateCachedOwnerIsNPC()
            // SkyrimSE.exe + 0x606850 [1.5.97.0]  [ADDRLIB:36889]
            // SkyrimSE.exe + 0x62de60 [1.6.318.0] [ADDRLIB:37913]
            // SkyrimSE.exe + 0x62dbd0 [1.6.323.0] [ADDRLIB:37913]
            constexpr REL::ID IsActorNPC(VERSION_SPECIFIC(36889, 37913));

            // SkyrimSE.exe + 0x6348a0 [1.5.97.0]  [ADDRLIB:37862]
            // SkyrimSE.exe + 0x65a970 [1.6.318.0] [ADDRLIB:38817]
            // SkyrimSE.exe + 0x65a6e0 [1.6.323.0] [ADDRLIB:38817]
            constexpr REL::ID
                GetRemainingSoulLevel(VERSION_SPECIFIC(37862, 38817));

            // SkyrimSE.exe + 0x634830 [1.5.97.0]  [ADDRLIB:37861]
            // SkyrimSE.exe + 0x65a900 [1.6.318.0] [ADDRLIB:38816] (inlined but not removed in source).
            // SkyrimSE.exe + 0x65a670 [1.6.323.0] [ADDRLIB:38816] (inlined but not removed in source).
            constexpr REL::ID
                GetRemainingSoulLevelValue(VERSION_SPECIFIC(37861, 38816));
        } // end namespace Actor

        // These addresses don't point to functions, but rather jmp points.
        // There is no actual stack being handled, thus it may be risky to call
        // these as if they were functions.
        //
        // Still, they're included here as documentation since they exist as
        // part of the other calls listed here.
        //
        // Signature: CalculateSoulLevel(uint32_t actorLevel, bool isNPC)
        // SkyrimSE.exe + 0x3c1740 [1.5.97.0]  [ADDRLIB:25933]
        // SkyrimSE.exe + 0x3d91a0 [1.6.318.0] [ADDRLIB:26520]
        // SkyrimSE.exe + 0x3d9110 [1.6.323.0] [ADDRLIB:26520]
        constexpr REL::ID CalculateSoulLevel(VERSION_SPECIFIC(25933, 26520));

        // Signature: ToSoulLevelValue(SOUL_LEVEL soulLevel)
        // SkyrimSE.exe + 0x237a90 [1.5.97.0]  [ADDRLIB:17753]
        // SkyrimSE.exe + 0x247e20 [1.6.318.0] [ADDRLIB:18166]
        // SkyrimSE.exe + 0x247d90 [1.6.323.0] [ADDRLIB:18166]
        constexpr REL::ID ToSoulLevelValue(VERSION_SPECIFIC(17753, 18166));
    } // namespace soultraputils

    namespace fix {
        namespace chargeitem {
            // This probably isn't updateInventory and may actually be part of
            // the update loop, but updating inventory is what we use it for
            // here.
            //
            // SkyrimSE.exe + 0x8d5710 [1.5.97.0]  [ADDRLIB:51911]
            // SkyrimSE.exe + 0x905cd0 [1.6.318.0] [ADDRLIB:52849]
            // SkyrimSE.exe + 0x905a40 [1.6.323.0] [ADDRLIB:52849]
            constexpr REL::ID updateInventory(VERSION_SPECIFIC(51911, 52849));

            // 0x2a5 [1.5.97.0]
            // 0x29b [1.6.318/323.0]
            constexpr std::ptrdiff_t patchOffset =
                VERSION_SPECIFIC(0x2a5, 0x29b);

            // Offset to return to when we finish the patched branch.
            // 0x2b9 [1.5.97.0]
            // 0x2af [1.6.318/323.0]
            constexpr std::ptrdiff_t continueOffset =
                VERSION_SPECIFIC(0x2b9, 0x2af);

            constexpr std::uintptr_t stackSize = 0xc8; // Same in AE
        } // namespace chargeitem

        namespace enchantitem {
            // 0x222 [1.5.97.0]
            // 0x220 [1.6.318/323.0]
            constexpr std::ptrdiff_t patchOffset =
                VERSION_SPECIFIC(0x222, 0x220);
            // Offset to return to when we finish the patched branch.
            // 0x236 [1.5.97.0]
            // 0x234 [1.6.318/323.0]
            constexpr std::ptrdiff_t continueOffset =
                VERSION_SPECIFIC(0x236, 0x234);

            constexpr std::uintptr_t stackSize = 0xb8; // Same in AE
        } // namespace enchantitem

        namespace trapsoul {
            // Part 1 of TrapSoul()
            // SkyrimSE.exe + 0x94d850 [1.5.97.0]  [ADDRLIB:53948]
            // SkyrimSE.exe + 0x97a270 [1.6.318.0] [ADDRLIB:54772]
            // SkyrimSE.exe + 0x97a190 [1.6.323.0] [ADDRLIB:54772]
            constexpr REL::ID TrapSoul0(VERSION_SPECIFIC(53948, 54772));

            // Part 2 of TrapSoul() (after jump)
            // SkyrimSE.exe + 0x634900 [1.5.97.0]  [ADDRLIB:37863]
            // SkyrimSE.exe + 0x65a9d0 [1.6.318.0] [ADDRLIB:38818]
            // SkyrimSE.exe + 0x65a740 [1.6.323.0] [ADDRLIB:38818]
            constexpr REL::ID TrapSoul1(VERSION_SPECIFIC(37863, 38818));

            constexpr std::ptrdiff_t patchOffset = 0x17; // Same in AE
            // 0x256 [1.5.97.0]
            // 0x282 [1.6.318/323.0]
            constexpr std::ptrdiff_t continueOffset =
                VERSION_SPECIFIC(0x256, 0x282);
        } // namespace trapsoul
    } // namespace fix
} // namespace re
