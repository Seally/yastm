#pragma once

#include <REL/Relocation.h>

#include "global.hpp"

// Note: The listed offsets in the comments only contain addresses for binaries
//       I've personally verified. Rely on the ADDRLIB ID to find addresses for
//       other binaries.
//
//       Skyrim VR address library IDs are generally identical to Skyrim SE v1.5
//       addresses because the addresses are added by hand.

namespace re {
    namespace SoulsTrapped {
        // SkyrimSE.exe + 0x636c40 [1.5.97.0]  [ADDRLIB:37916]
        // SkyrimSE.exe + 0x65c9e0 [1.6.318.0] [ADDRLIB:38873]
        // SkyrimSE.exe + 0x65c750 [1.6.323.0] [ADDRLIB:38873]
        // SkyrimVR.exe + 0x63fc50 [1.4.15.1]  [ADDRLIB:37916]
        constexpr REL::ID GetEventSource(VERSION_SPECIFIC_2(37916, 38873));
    } // namespace SoulsTrapped

    namespace BSExtraDataList {
// #if defined(SKYRIM_VERSION_AE2)
//         constexpr REL::ID Constructor(11583);
// #endif
        // SkyrimSE.exe + 0x10e960 [1.5.97.0]  [ADDRLIB:11474]
        // SkyrimSE.exe + 0x11af60 [1.6.323.0] [ADDRLIB:11620]
        // SkyrimVR.exe + 0x126c00 [1.4.15.1]  [ADDRLIB:11474]
        constexpr REL::ID SetSoul(VERSION_SPECIFIC_2(11474, 11620));
    } // namespace BSExtraDataList

    namespace CraftingSubMenus {
        namespace EnchantMenu {
            // CraftingSubMenus::EnchantMenu::EnchantItem
            //
            // SkyrimSE.exe + 0x86c640 [1.5.97.0]  [ADDRLIB:50450]
            // SkyrimSE.exe + 0x89a9c0 [1.6.318.0] [ADDRLIB:51355]
            // SkyrimSE.exe + 0x89a730 [1.6.323.0] [ADDRLIB:51355]
            // SkyrimVR.exe + 0x897bd0 [1.4.15.1]  [ADDRLIB:50450]
            constexpr REL::ID EnchantItem(VERSION_SPECIFIC_2(50450, 51355));
        } // namespace EnchantMenu
    } // namespace CraftingSubMenus

    namespace InventoryMenu {
        // SkyrimSE.exe + 0x88e890 [1.5.97.0]  [ADDRLIB:50980]
        // SkyrimSE.exe + 0x8bdfe0 [1.6.318.0] [ADDRLIB:51859]
        // SkyrimSE.exe + 0x8bdd50 [1.6.323.0] [ADDRLIB:51859]
        // SkyrimVR.exe + 0x8bc060 [1.4.15.1]  [ADDRLIB:50980]
        constexpr REL::ID ChargeItem(VERSION_SPECIFIC_2(50980, 51859));
    } // namespace InventoryMenu

    namespace Actor {
        // SkyrimSE.exe + 0x634900 [1.5.97.0]  [ADDRLIB:37863]
        // SkyrimSE.exe + 0x65a9d0 [1.6.318.0] [ADDRLIB:38818]
        // SkyrimSE.exe + 0x65a740 [1.6.323.0] [ADDRLIB:38818]
        // SkyrimVR.exe + 0x63d7d0 [1.4.15.1]  [ADDRLIB:37863]
        constexpr REL::ID TrapSoul(VERSION_SPECIFIC_2(37863, 38818));
    } // namespace Actor

    namespace papyrus {
        namespace Actor {
            // SkyrimSE.exe + 0x94d850 [1.5.97.0]  [ADDRLIB:53948]
            // SkyrimSE.exe + 0x97a270 [1.6.318.0] [ADDRLIB:54772]
            // SkyrimSE.exe + 0x97a190 [1.6.323.0] [ADDRLIB:54772]
            // SkyrimVR.exe + 0x987ab0 [1.4.15.1]  [ADDRLIB:53948]
            constexpr REL::ID TrapSoul(VERSION_SPECIFIC_2(53948, 54772));
        } // namespace Actor
    } // namespace papyrus

    namespace soultraputils {
        namespace Actor {
            // Possibly related to/same as Actor::CalculateCachedOwnerIsNPC()
            // SkyrimSE.exe + 0x606850 [1.5.97.0]  [ADDRLIB:36889]
            // SkyrimSE.exe + 0x62de60 [1.6.318.0] [ADDRLIB:37913]
            // SkyrimSE.exe + 0x62dbd0 [1.6.323.0] [ADDRLIB:37913]
            // SkyrimVR.exe + 0x60f1d0 [1.4.15.1]  [ADDRLIB:36889]
            constexpr REL::ID IsActorNPC(VERSION_SPECIFIC_2(36889, 37913));

            // SkyrimSE.exe + 0x6348a0 [1.5.97.0]  [ADDRLIB:37862]
            // SkyrimSE.exe + 0x65a970 [1.6.318.0] [ADDRLIB:38817]
            // SkyrimSE.exe + 0x65a6e0 [1.6.323.0] [ADDRLIB:38817]
            // SkyrimVR.exe + 0x63d770 [1.4.15.1]  [ADDRLIB:37862]
            constexpr REL::ID
                GetRemainingSoulLevel(VERSION_SPECIFIC_2(37862, 38817));

            // SkyrimSE.exe + 0x634830 [1.5.97.0]  [ADDRLIB:37861]
            // SkyrimSE.exe + 0x65a900 [1.6.318.0] [ADDRLIB:38816] (inlined but not removed in source).
            // SkyrimSE.exe + 0x65a670 [1.6.323.0] [ADDRLIB:38816] (inlined but not removed in source).
            // SkyrimVR.exe + 0x63d700 [1.4.15.1]  [ADDRLIB:37861]
            constexpr REL::ID
                GetRemainingSoulLevelValue(VERSION_SPECIFIC_2(37861, 38816));
        } // end namespace Actor

        // We don't use these addresses directly, though we keep them here
        // for documentation purposes.
        //
        // Signature: CalculateSoulLevel(uint32_t actorLevel, bool isNPC)
        // SkyrimSE.exe + 0x3c1740 [1.5.97.0]  [ADDRLIB:25933]
        // SkyrimSE.exe + 0x3d91a0 [1.6.318.0] [ADDRLIB:26520]
        // SkyrimSE.exe + 0x3d9110 [1.6.323.0] [ADDRLIB:26520]
        // SkyrimVR.exe + 0x3d0fc0 [1.4.15.1]  [ADDRLIB:???]

        // Signature: ToSoulLevelValue(SOUL_LEVEL soulLevel)
        // SkyrimSE.exe + 0x237a90 [1.5.97.0]  [ADDRLIB:17753]
        // SkyrimSE.exe + 0x247e20 [1.6.318.0] [ADDRLIB:18166]
        // SkyrimSE.exe + 0x247d90 [1.6.323.0] [ADDRLIB:18166]
        // SkyrimVR.exe + 0x249160 [1.4.15.1]  [ADDRLIB:???]
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
            // SkyrimVR.exe + 0x903370 [1.4.15.1]  [ADDRLIB:51911]
            constexpr REL::ID updateInventory(VERSION_SPECIFIC_2(51911, 52849));

            // 0x2a5 [SE-1.5.97.0]
            // 0x29b [AE-1.6.318/323.0]
            // 0x2a5 [VR-1.4.15.1]
            constexpr std::ptrdiff_t patchOffset =
                VERSION_SPECIFIC_2(0x2a5, 0x29b);

            // Offset to return to when we finish the patched branch.
            // 0x2b9 [SE-1.5.97.0]
            // 0x2af [AE-1.6.318/323.0]
            // 0x2b9 [VR-1.4.15.1]
            constexpr std::ptrdiff_t continueOffset =
                VERSION_SPECIFIC_2(0x2b9, 0x2af);
        } // namespace chargeitem

        namespace enchantitem {
            // 0x222 [SE-1.5.97.0]
            // 0x220 [AE-1.6.318/323.0]
            // 0x222 [VR-1.4.15.1]
            constexpr std::ptrdiff_t patchOffset =
                VERSION_SPECIFIC_2(0x222, 0x220);
            // Offset to return to when we finish the patched branch.
            // 0x236 [SE-1.5.97.0]
            // 0x234 [AE-1.6.318/323.0]
            // 0x236 [VR-1.4.15.1]
            constexpr std::ptrdiff_t continueOffset =
                VERSION_SPECIFIC_2(0x236, 0x234);
        } // namespace enchantitem

        namespace trapsoul {
            constexpr std::ptrdiff_t branchJmpOffset = 0x6; // Same in AE
        } // namespace trapsoul
    } // namespace fix
} // namespace re

#undef RELOCATION_ID
